#!/bin/bash -x
#
# Script to be invoked by "git bisect" to automate the bisect
# process of finding the commit that breaks the code. It requires
# an RSA token to be installed. 
#
# Usage: 
#      $ git bisect start HEAD <last_known_good_commit>
#      $ git bisect run ./test-error.sh
#
# Later I'll have this script accept parameters, so user could tell it that
# the token is RSA or EC (and thus use the appropriate commands).
#


# PIN for the token - treat with care!
PIN="xxxxxx"

# When multiple readers are installed - SLOT reference may be needed. In that case
# uncomment the 2nd "P11" line (and comment out the 1st one :). Also edit the following
# line to point at the correct slot.
SLOT=1

# OpenSSL command line executable and parameters
OPENSSL="/opt/local/bin/openssl "
PRKEYGENRSA="genpkey -algorithm RSA -outform DER -out rsa2048priv.der -pkeyopt rsa_keygen_bits:2048"
PRKEYGENEC="genpkey -algorithm EC -outform DER -out ec256priv.der -pkeyopt ec_paramgen_curve:prime256v1"
PUBKEYRSA="rsa -pubout -inform DER -outform DER -in rsa2048priv.der -out rsa2048pub.der"
PUBKEYEC="ec -inform DER -in ec256priv.der -pubout -outform DER -out ec256pub.der"
GENRAND="rand -out t256.dat 32"
OPENSSLRSAENCR="rsautl -encrypt -pkcs -keyform PEM -pubin -inkey token03pub.pem -in t256.dat -out t256.dat.enc"

# Set key pair generation to RSA or EC
KEYTYPE="RSA"
#KEYTYPE="EC"

# Install directory
INSTDIR="/tmp/OpenSC"

# OpenSC PKCS11 library
MODULE="${INSTDIR}/lib/opensc-pkcs11.so"
# pkcs15-tool invocation (used to retrieve public key or certificate from the token)
P15="env DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${INSTDIR}/lib ${INSTDIR}/bin/pkcs15-tool "
# pkcs15-tool parameters to retrieve "KEY MAN pubkey" (id 03) from the token
P15GETKEY=" --read-public-key 03 -o token03pub.pem"
# pkcs11-tool invocation command
P11="env DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${INSTDIR}/lib ${INSTDIR}/bin/pkcs11-tool --module ${MODULE} "
# Comment the above line and uncomment the line below if you need to specify the slot/reader where the token is
#P11="env DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${INSTDIR}/lib ${INSTDIR}/bin/pkcs11-tool --module ${MODULE} --slot ${SLOT} "
# pkcs11-tool parameters to decrypt (OpenSSL-encrypted) test-data to 
P11RSADECR="--pin ${PIN} -m RSA-PKCS --decrypt --id 03 -i t256.dat.enc -o t256.dat.dec"

# Clean the old stuff
rm -rf /tmp/OpenSC
# Make sure the directory exists
mkdir -p /tmp/OpenSC

# No need to touch the local repo, as "git bisect" takes care of that

# Just in case, re-create RSA and EC key pairs
${OPENSSL} ${PRKEYGENRSA}
${OPENSSL} ${PUBKEYRSA}
${OPENSSL} ${PRKEYGENEC}
${OPENSSL} ${PUBKEYEC}
${OPENSSL} ${GENRAND}

# Re-do the configuration because commits could affect it
./configure --prefix=/tmp/OpenSC
# Build and install into /tmp/OpenSC
make install

# Now - the actual test-run

# Retrieve KEY MAN pubkey
${P15} ${P15GETKEY}
if [ $? != 0 ]; then
	echo "Failed to retrieve \"KEY MAN pubkey\" from token"
	exit 1
fi
# Encrypt test-data so pkcs11-tool can decrypt it
${OPENSSL} ${OPENSSLRSAENCR}
if [ $? != 0 ]; then
	echo "OpenSSL failed to encrypt t.256.dat using token03pub.pem"
	exit 1
fi
# Attempt to decrypt the above test-data
${P11} ${P11RSADECR}
if [ $? != 0 ]; then
	echo "pkcs11-tool failed to decrypt t256.dat.enc"
	exit 1
fi

# Compare decrypted data with the original plaintext
R=`cmp t256.dat t256.dat.dec`
if [ $? != 0 ]; then
	echo "Decrypted data does not match the original plaintext!"
	exit 1
fi

# Cleanup (we can leave the generated keypairs, and they'd be overwritten anyway)
rm -f t256.dat.dec t256.dat.enc
rm -f token03pub.pem

# If we are here, everything above completed successfully
# So we can declare the build of this commit a success
exit 0
