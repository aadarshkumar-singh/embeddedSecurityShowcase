import sys

from cysecuretools import CySecureTools
tools = CySecureTools('CY8CPROTO-064S1-SB')
tools.sign_image(sys.argv[1])
