from cysecuretools import CySecureTools
tools = CySecureTools('CY8CPROTO-064S1-SB', 'policy/policy_single_stage_CM4.json')
tools.create_provisioning_packet();