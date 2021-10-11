import os
import shutil
import cysecuretools
os.makedirs('policy') if not os.path.exists('policy') else None
shutil.copy2(os.path.join(os.path.dirname(cysecuretools.__file__), 'targets/cy8cproto_064s1_sb/policy/policy_single_stage_CM4.json'), './policy/policy_single_stage_CM4.json')