#!/bin/bash -x
#
# (C) 2017 Mouse
#
# Script to be invoked by "git bisect" to automate the bisect
# process of finding the commit that breaks the code. It requires
# an RSA token to be installed.
#
# Script Usage: ./test-error.sh [<file_where_PIN_is_stored> ["EC"]]
#      - first parameter is self-explanatory. You need to either hard-code the PIN in
#        this script, or put it in a file and give that file as the first parameter.
#      - if you want to run the script with an ECC-based token, you have to provide
#        the first parameter (i.e., hard-coding the PIN in the script won't work any
#        more), and give the string "EC" as the second parameter.
#
# Intended Usage: 
#      $ git bisect start HEAD <last_known_good_commit>
#      $ git bisect run ./test-error.sh <file_with_PIN> ["EC"]
#


# PIN for the token - treat with care!
PIN="xxxxxx"
if (( $# > 0 )); then
    # Set PIN from a file
    PIN=`cat $1`
fi

# When multiple readers are installed - SLOT reference may be needed. In that case
# uncomment the 2nd "P11" line (and comment out the 1st one :). Also edit the following
# line to point at the correct slot.
SLOT=1

# OpenSSL command line executable and parameters
OPENSSL=`which openssl`
PRKEYGENRSA="genpkey -algorithm RSA -outform DER -out rsa2048priv.der -pkeyopt rsa_keygen_bits:2048"
PRKEYGENEC="ecparam -name prime256v1 -genkey -out ec256priv.pem"
PUBKEYRSA="rsa -pubout -inform DER -outform DER -in rsa2048priv.der -out rsa2048pub.der"
PUBKEYEC="ec -in ec256priv.pem -pubout -outform DER -out ec256pub.der"
GENRAND="rand -out t256.dat 32"
OPENSSLRSAENCR="rsautl -encrypt -pkcs -keyform PEM -pubin -inkey token03pub.pem -in t256.dat -out t256.dat.enc"
OPENSSLECDERIVE="pkeyutl -derive -inkey ec256priv.pem -peerkey token03pub.pem -out t256-derived1.dat"

# Set common variables that would control choosing between RSA and ECC tokens
OSSLRSAERR="OpenSSL failed to encrypt t.256.dat using token03pub.pem"
P11RSAERR="pkcs11-tool failed to decrypt t256.dat.enc"
OSSLECERR="OpenSSL failed to derive secret from ephemeral EC priv key and token03pub.pem"
P11ECERR="pkcs11-tool failed to derive secret from EC priv key on token and ephemeral pub key ec256pub.der"
CMPRSAERR="Decrypted data does not match the original plaintext!"
CMPECERR="Derived secrets do not match!"
CMPERR="${CMPRSAERR}"

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
P11ECDERIVE="--pin ${PIN} --derive -m ECDH1-COFACTOR-DERIVE --id 03 -i ec256pub.der -o t256-derived2.dat"

P11RSA="${P11} ${P11RSADECR}"
P11EC="${P11}  ${P11ECDERIVE}"

P11CALL="${P11RSA}"
OCALL="${OPENSSL} ${OPENSSLRSAENCR}"
P11ERR="${P11RSAERR}"
OERR="${OSSLRSAERR}"

DOINGEC="false"
if (( $# > 1 )); then
    if [ "$2" = "EC" ]; then
	# Working with EC token instead of RSA
	P11CALL="${P11EC}"
	P11ERR="${P11ECERR}"
	OCALL="${OPENSSL} ${OPENSSLECDERIVE}"
	OERR="${OSSLECERR}"
	CMPERR="${CMPECERR}"
	DOINGEC="true"
    fi
fi


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
# Encrypt test-data so pkcs11-tool can decrypt it (or for EC - derive using ephem privkey and pubkey from token)
#${OPENSSL} ${OPENSSLRSAENCR}
${OCALL}
if [ $? != 0 ]; then
	echo "${OERR}"
	exit 1
fi
# Attempt to decrypt the above test-data (or for EC - derive using privkey on token and ephem pubkey)
#${P11} ${P11RSADECR}
${P11CALL}
if [ $? != 0 ]; then
	echo "${P11ERR}"
	exit 1
fi

# Compare decrypted data with the original plaintext

if [ "${DOINGEC}" = "true" ]; then
    R=`cmp t256-derived1.dat t256-derived2.dat`
else
    R=`cmp t256.dat t256.dat.dec`
fi
if [ $? != 0 ]; then
	echo "${CMPERR}"
	exit 1
fi

# Cleanup (we can leave the generated keypairs, and they'd be overwritten anyway)
rm -f t256.dat t256.dat.dec t256.dat.enc t256-derived*.dat
rm -f ec256priv.pem ec256pub.pem ec256priv.der ec256pub.der
rm -f rsa2048priv.pem rsa2048pub.pem rsa2048priv.der rsa2048pub.der
rm -f token03pub.pem

# If we are here, everything above completed successfully
# So we can declare the build of this commit a success
echo "Everything OK"
exit 0
