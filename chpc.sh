#
# chpc: compile grammatical channel protocol.
#
#          input: $1 == file containing protocol description.
#
#          output: executable protocol program created in p.out;
#		   #defines for message types placed into pdefs.h
#
input=$1
PATH=`logdir portegys`/bin:$PATH
if [ -z "$input" ]
then
	echo Usage: $0 protocol-description-file
	exit 1
fi
if [ $# -ne 1 ]
then
	echo Usage: $0 protocol-description-file
	exit 1
fi
if [ ! -r $input ]
then
	echo $0: cannot open $input
	exit 1
fi
trap '
	rm -fr /usr/tmp/chpc.$$ 2>/dev/null
	exit 1
' 1 2 15
mkdir /usr/tmp/chpc.$$

# create user yacc program
cat $input | comout | chpcx >/usr/tmp/chpc.$$/pr.yacc
if [ $? -ne 0 ]
then
	exit 1
fi

olddir=`pwd`
cd /usr/tmp/chpc.$$

# run yacc on user program
yacc -d pr.yacc
if [ $? -ne 0 ]
then
	exit 1
fi

cp `logdir portegys`/gramch/chlex.c .
cp `logdir portegys`/gramch/chdefs.h .

#compile user protocol
cc y.tab.c >/dev/null 2>errs
ret=$?
cat errs | grep -v preroot
if [ $ret -ne 0 ]
then
	exit 1
fi

mv a.out $olddir/p.out
mv y.tab.h $olddir/pdefs.h
cd
rm -fr /usr/tmp/chpc.$$
exit 0
