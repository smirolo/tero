#!/usr/bin/env python
#
# Copyright (c) 2009, Sebastien Mirolo
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

#   THIS SOFTWARE IS PROVIDED BY Sebastien Mirolo ''AS IS'' AND ANY
#   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL Sebastien Mirolo BE LIABLE FOR ANY
#   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import re, os, StringIO, sys, subprocess, tempfile, xml.sax

licensePats = {
    'BSD': '''\S?\S?\s*Copyright \(c\) (?P<date>(\d+)(-\d+)*), (?P<grantor>.*)
\S?\s*All rights reserved.
\S?\s*
\S?\s*Redistribution and use in source and binary forms, with or without
\S?\s*modification, are permitted provided that the following conditions are met:
\S?\s*\* Redistributions of source code must retain the above copyright
\S?\s*notice, this list of conditions and the following disclaimer.
\S?\s*\* Redistributions in binary form must reproduce the above copyright
\S?\s*notice, this list of conditions and the following disclaimer in the
\S?\s*documentation and/or other materials provided with the distribution.
\S?\s*\* Neither the name of (?P<brand>.*) nor the
\S?\s*names of its contributors may be used to endorse or promote products
\S?\s*derived from this software without specific prior written permission.

\S?\s*THIS SOFTWARE IS PROVIDED BY (.*) ''AS IS'' AND ANY
\S?\s*EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
\S?\s*WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
\S?\s*DISCLAIMED. IN NO EVENT SHALL (.*) BE LIABLE FOR ANY
\S?\s*DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
\S?\s*\(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
\S?\s*LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION\) HOWEVER CAUSED AND
\S?\s*ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
\S?\s*\(INCLUDING NEGLIGENCE OR OTHERWISE\) ARISING IN ANY WAY OUT OF THE USE OF THIS
\S?\s*SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE\..*
'''
}

def license(source,license):
    sourceLine = source.readline()
    licenseLine = license.readline()
    inprogress = False
    fields = { 'brand': None, 'grantor': None, 'date': None }
    while sourceLine != '' and licenseLine != '':
        look = re.match(licenseLine.strip(),sourceLine.strip())
        if look:
            inprogress = True
            for p in look.groupdict():
                fields[p] = look.groupdict()[p]
            licenseLine = license.readline()
        elif inprogress:
            break
        sourceLine = source.readline()
    if licenseLine == '':
        # Whole license has been read.
        return True, fields
    return False, fields


def findLicense(filename):
    s = open(filename)
    for l in licensePats:
        lic, fields = license(s,StringIO.StringIO(licensePats[l]))
        if lic:
            return l, fields
    s.close()
    return None, {}

def findAllLicenses(base):
    if os.path.isfile(base):
            sys.stdout.write(base + '... ')
            lic, fields = findLicense(base)
            if lic:
                sys.stdout.write(lic + ' (' + fields['brand'] \
                                     + ', ' + fields['date'] \
                                     + ' by ' + fields['grantor'] + ')\n')
            else:
                sys.stdout.write('no or unknown license\n')
            # reportStyleBreakage(base)
    elif os.path.isdir(base):
        for p in os.listdir(base):            
            findAllLicenses(os.path.join(base,p))

class styleBreakageHandler(xml.sax.ContentHandler):

    def __init__(self):
        self.identifier = None
        self.identifiers = []

    def startElement(self, name, attrs):
        self.identifier = None
        if name == 'span':
            if 'class' in attrs and attrs['class'] == 'unstyledIdentifier':
                self.identifier = ''

    def characters(self, ch):
        if self.identifier != None:
            self.identifier = self.identifier + ch

    def endElement(self, name):
        if self.identifier != None:
            if not self.identifier in self.identifiers:
                self.identifiers += [ self.identifier ] 
            self.identifier = None 

def reportStyleBreakage(base):
    basename, ext = os.path.splitext(base)
    if ext in [ '.cc', '.c', '.hh', '.h' ]:
        if not re.match('([a-z,0-9]+([A-Z][a-z,0-9]+)*)',basename):
            sys.stdout.write(base + ' does not match filename style\n')
        style = styleBreakageHandler()
        parser = xml.sax.make_parser()
        parser.setFeature(xml.sax.handler.feature_namespaces, 0)
        parser.setContentHandler(style)
        cmdline = [ 'seed', base ]
        tmpout = tempfile.NamedTemporaryFile(delete=False)
        print "write output in " + tmpout.name + "..."
        cmd = subprocess.Popen(cmdline,
                               stdout=subprocess.PIPE)
        written = False
        line = cmd.stdout.readline()
        while line != '':
            if line.startswith('<'):
                written = True
            if written:
                tmpout.write(line)
            line = cmd.stdout.readline()
        cmd.wait()
        if cmd.returncode != 0:
            raise "Command returned error"
        tmpout.close()
        parser.parse(tmpout.name)
        for identifier in style.identifiers:
            sys.stdout.write(identifier + '\n')


# Main Entry Point
if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.stderr.write('usage: ' + sys.argv[0] + ' root');
        sys.exit(1)
    findAllLicenses(sys.argv[1])
