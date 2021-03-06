#!/usr/bin/env python
#
# This file is part of pyasn1-modules software.
#
# Copyright (c) 2005-2017, Ilya Etingof <etingof@gmail.com>
# License: http://pyasn1.sf.net/license.html
#
# Read ASN.1/PEM X.509 CRMF request on stdin, parse into
# plain text, then build substrate from it
#
from pyasn1.codec.der import decoder, encoder
from pyasn1_modules import rfc2511, pem
import sys

if len(sys.argv) != 1:
    print("""Usage:
$ cat crmf.pem | %s""" % sys.argv[0])
    sys.exit(-1)
    
certReq = rfc2511.CertReqMessages()

substrate = pem.readBase64FromFile(sys.stdin)
if not substrate:
    sys.exit(0)
        
cr, rest = decoder.decode(substrate, asn1Spec=certReq)

print(cr.prettyPrint())

assert encoder.encode(cr) == substrate, 'crmf recode fails'
