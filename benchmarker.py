# Run the file an appropriate number of times
import timeit
from statistics import mean, stdev
import matplotlib.pyplot as plt

# run the python XMSS

# keygen
setup = '''
from xmss.core.xmss import XMSS
from xmss.otss.winternitz import WinternitzOTS
'''

param_w = [2, 4, 8]
param_h = [6, 8, 10]

for w in param_w:
    for h in param_h:
        time = timeit.repeat(f'''
ots_scheme = WinternitzOTS(w={w})
xmss = XMSS(ots_scheme=ots_scheme, height={h})

private_key, public_key = xmss.generate_keypair()
''', setup, repeat=20, number=1)
        print(f"Keygen - w = {w}, h = {h}")
        print("min:", min(time))
        print("mean:", mean(time))
        print("max:", max(time))
        print("sd:", stdev(time))

# signing
for w in param_w:
    for h in param_h:
        setup = f'''
from xmss.core.xmss import XMSS
from xmss.otss.winternitz import WinternitzOTS

ots_scheme = WinternitzOTS(w={w})
xmss = XMSS(ots_scheme=ots_scheme, height={h})

private_key, public_key = xmss.generate_keypair()
'''

        time = timeit.repeat(f'''
message = b"Hello, XMSS!"
signature = xmss.sign(message)
''', setup, repeat=20, number=1)
        print(f"Signing - w = {w}, h = {h}")
        print("min:", min(time))
        print("mean:", mean(time))
        print("max:", max(time))
        print("sd:", stdev(time))

# signing
for w in param_w:
    for h in param_h:
        setup = f'''
from xmss.core.xmss import XMSS
from xmss.otss.winternitz import WinternitzOTS

ots_scheme = WinternitzOTS(w={w})
xmss = XMSS(ots_scheme=ots_scheme, height={h})

private_key, public_key = xmss.generate_keypair()

message = b"Hello, XMSS!"
signature = xmss.sign(message)
'''

        time = timeit.repeat(f'''
is_valid = xmss.verify(message, signature)
''', setup, repeat=20, number=1)
        print(f"Verifying - w = {w}, h = {h}")
        print("min:", min(time))
        print("mean:", mean(time))
        print("max:", max(time))
        print("sd:", stdev(time))

# plt.hist(time, density=True)
# plt.show()