/**
 * Python SCSI generic library
 *
 * Copyright (C) 2008-2026 by Daniel Lenski <lenski@umd.edu>
 *
 * Migrated to Python3 by tvladyslav <ykp@protonmail.ch>
 *
 * Released under the terms of the
 * GNU General Public License version 3 or later
 */

#define PY_SSIZE_T_CLEAN

#include <Python.h>

#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>

static PyObject *SCSIError;
PyDoc_STRVAR(SCSIError__doc__,
"SCSI operation failed.\n\n"
"The args are a 5-tuple containing the masked_status,\n"
"host_status, driver_status, sense buffer, and data buffer fields\n"
"from the failed operation:\n\n"
"SCSIError(masked_status, host_status, driver_status, sense, buf)");

static int
obj_to_fd(PyObject *object, int *target)
{
    int fd = PyObject_AsFileDescriptor(object);

    if (fd < 0)
        return 0;
    *target = fd;
    return 1;
}

//////////////////////////////////////////////////////////////////////

PyDoc_STRVAR(write__doc__,
"write(sg_fd, cmd, buf=None, timeout_ms=20000, flags=0) -> None\n\n"
"Issue a command and write data.  Returns nothing.");

static PyObject *
sg_write(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int sg_fd;
    unsigned int timeout=20000, flags=0;
    uint8_t *cmd, *buf=NULL;
    Py_ssize_t cmdLen, bufLen=0;

    // parse and check arguments
    static char *kwlist[] = {"fd", "cmd", "buf", "timeout_ms", "flags", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&y#|y#II:write", kwlist,
                                     obj_to_fd, &sg_fd, &cmd, &cmdLen, &buf, &bufLen, &timeout, &flags))
        return NULL;

    // submit SG_IO ioctl

    sg_io_hdr_t io;
    uint8_t sense[32];
    int r;

    memset(&io, 0, sizeof(io));

    io.interface_id = 'S';
    io.cmd_len = cmdLen;
    /* io.iovec_count = 0; */  /* memset takes care of this */
    io.mx_sb_len = sizeof(sense);
    io.dxfer_direction = SG_DXFER_TO_DEV;
    io.dxfer_len = bufLen;
    io.dxferp = buf;
    io.cmdp = cmd;
    io.sbp = sense;
    io.timeout = timeout;   /* in millisecs */
    io.flags = flags;
    /* io.pack_id = 0; */
    /* io.usr_ptr = NULL; */

    r = ioctl(sg_fd, SG_IO, &io);

    // handle errors

    if (r < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
        return NULL;
    } else if ((io.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
        PyErr_SetObject(SCSIError,
                        Py_BuildValue("BBBy#y#", io.masked_status, io.host_status, io.driver_status,
                            (io.sb_len_wr > 0 ? sense : NULL), io.sb_len_wr, buf, bufLen));
        return NULL;
    }

    Py_RETURN_NONE;
}

//////////////////////////////////////////////////////////////////////

PyDoc_STRVAR(read__doc__,
"read(sg_fd, cmd, bufLen, timeout_ms=20000, flags=0) -> bytes\n\n"
"Issue a command and read a response.\n"
"Response is returned as bytes.");

static PyObject *
sg_read(PyObject *self, PyObject *args, PyObject *kwargs)
{
    int sg_fd;
    unsigned int timeout=20000, flags=0;
    uint8_t *cmd, *buf;
    Py_ssize_t cmdLen, bufLen;
    PyObject *bufObj;

    // parse and check arguments

    static char *kwlist[] = {"fd", "cmd", "bufLen", "timeout_ms", "flags", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&y#n|II:read", kwlist,
                                     obj_to_fd, &sg_fd, &cmd, &cmdLen, &bufLen, &timeout, &flags))
        return NULL;

    bufObj = PyBytes_FromStringAndSize(NULL, bufLen); // new blank bytes
    if (!bufObj) return NULL;
    buf = (unsigned char*)PyBytes_AS_STRING(bufObj);

    // submit SG_IO ioctl

    sg_io_hdr_t io;
    uint8_t sense[32];
    int r;

    memset(&io, 0, sizeof(io));

    io.interface_id = 'S';
    io.cmd_len = cmdLen;
    /* io.iovec_count = 0; */  /* memset takes care of this */
    io.mx_sb_len = sizeof(sense);
    io.dxfer_direction = SG_DXFER_FROM_DEV;
    io.dxfer_len = bufLen;
    io.dxferp = buf;
    io.cmdp = cmd;
    io.sbp = sense;
    io.timeout = timeout;   /* in millisecs */
    io.flags = flags;
    /* io.pack_id = 0; */
    /* io.usr_ptr = NULL; */

    r = ioctl(sg_fd, SG_IO, &io);

    // handle errors
    if (r < 0) {
        PyErr_SetFromErrno(PyExc_OSError);
    } else {
        // trim to size of data actually received
        const Py_ssize_t len = io.dxfer_len - io.resid;
        if (_PyBytes_Resize(&bufObj, len) < 0) return NULL;

        if ((io.info & SG_INFO_OK_MASK) != SG_INFO_OK)
            PyErr_SetObject(SCSIError,
                            Py_BuildValue("BBBy#O", io.masked_status, io.host_status, io.driver_status,
                                (io.sb_len_wr > 0 ? sense : NULL), io.sb_len_wr, bufObj));
        else
            return bufObj;
    }

    return NULL;
}

//////////////////////////////////////////////////////////////////////

PyDoc_STRVAR(module__doc__,
"This module issues commands to SCSI devices under Linux,\n"
"via the SG_IO ioctl of the scsi_generic driver.\n"
"\n"
"The device (e.g. /dev/sg0, /dev/sda, etc.) must first be\n"
"opened as an unbuffered binary file, and the file object or\n"
"descriptor passed to the methods of this module.");

static PyMethodDef SgMethods[] = {
    {"write", (PyCFunction)sg_write, METH_VARARGS | METH_KEYWORDS, write__doc__},
    {"read",  (PyCFunction)sg_read,  METH_VARARGS | METH_KEYWORDS, read__doc__},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef py_sg_definition = {
        PyModuleDef_HEAD_INIT,
        "py_sg",
        module__doc__,
        -1,
        SgMethods
};

PyMODINIT_FUNC
PyInit_py_sg(void)
{
    // initialize module
    Py_Initialize();
    PyMODINIT_FUNC mod = PyModule_Create(&py_sg_definition);
    if (!mod) return NULL;

    // SCSIError
    PyObject *doc = Py_BuildValue("{ss}", "__doc__", SCSIError__doc__);
    SCSIError = PyErr_NewException( "py_sg.SCSIError", NULL, doc);

    PyModule_AddObject(mod, "SCSIError", SCSIError);
    return mod;
}
