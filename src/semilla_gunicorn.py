#!/usr/bin/env python
#
# Copyright (c) 2009-2012, Fortylines LLC
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of fortylines nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY Fortylines LLC ''AS IS'' AND ANY
#   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL Fortylines LLC BE LIABLE FOR ANY
#   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import subprocess

def app(environ, start_response):
    '''We use gunicorn as a wrapper since nginx does not support CGI.'''
    cmdenv = {}
    for key, value in environ.iteritems():
        if isinstance(value, basestring):
            cmdenv.update({key: value})
    cmdenv.update({ 'SCRIPT_FILENAME': '/usr/bin/semilla' })
    body = environ["wsgi.input"]

    cmdline = [ '/usr/bin/semilla',
                '--config=/etc/semilla/fortylines.com.conf',
                environ['PATH_INFO'] ]
    cmd = subprocess.Popen(cmdline,
                           stdin=subprocess.PIPE,
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE,
                           env=cmdenv)
    output, errors = cmd.communicate(body.read())
    cmd.wait()
    if cmd.returncode != 0:
        status = '500 Internal server error'
    else:
        status = '200 OK'
    response_headers = []
    output = output.splitlines()
    line = output.pop(0)
    while line:
        header = tuple(line.split(':'))
        if header[0] == 'Status':
            status = header[1]
        else:
            response_headers.append(header)
        line = output.pop(0)
#   response_headers.append(('Content-Length', str(len('\n'.join(output)))))
    start_response(status, response_headers)
    return iter(output)
