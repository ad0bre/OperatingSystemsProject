if [ $# -ne 1 ]
then
 echo "usage: $0 <c>"
 exit 1
fi

count=0

#this loop reads lines from stdin until EOF or CTRL + D and counts the number of lines that contain the character c and respect the following format: the line starts with a capital letter, cointains only small letters, digits, spaces, commas and ends with a dot and there is never a comma before the word 'si'
while read line
do
 if [ `echo "$line" | grep -E "^[A-Z][a-z0-9 ,]*[!?\.]$" | grep -c "$1"` -gt 0 ]
 then
  count=`expr $count + 1`
 fi
done

echo "$count"