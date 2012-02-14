#!/bin/sh
#
#
#

PATH=${PATH}:.:/tmp
# edit this: serial port of the roomba
PORT="/dev/usb/tts/0"
# edit this: path to roombacmd.mpl
ROOMBACMD="/tmp/roombacmd"
# edit this: where the webcam is writing an image
PICPATH="/tmp/SpcaPic.jpg"
# edit this: where archived ("snapshot") images should be stored
SNAPPATH="/mydisk/archive/cam-`date -Is`"

me="$SCRIPT_NAME"
cmd="$QUERY_STRING"

cat <<EOF
Content-type: text/html

<html>
<head>
<title> Roomba Control Panel </title>
<link href="roombapanel.css" rel="stylesheet" type="text/css">
</head>
<body>
<center>
<h2>Roomba Control Panel</h2>

<table border=0>
<tr><td><a href="$me?init">init</a></td></td> 
    <td><a href="$me?sensors">sensors</a></td>
    <td><a href="$me?spy">spy</a></td></tr>
<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr><td>&nbsp;</td> 
    <td><a href="$me?forward">forward</a></td>
    <td>&nbsp;</td></tr>
<tr><td><a href="$me?spinleft">spinleft</a></td>
    <td><a href="$me?stop">stop</a></td> 
    <td><a href="$me?spinright">spinright</a></td></tr>
<tr><td>&nbsp;</td> 
   <td><a href="$me?backward">backward</a></td>
   <td>&nbsp;</td></tr>
<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr><td>&nbsp;</td>
    <td>&nbsp;</td>
    <td><a href="$me?snapshot">snapshot</a></td></tr>

</table>
</center>
<pre>
EOF

echo -n "Date: "; date

if [ "$cmd" ] ; then
    echo "cmd: $cmd"
    if [ "$cmd" = "snapshot" ] ; then
        echo "saving image and sensors to $SNAPPATH!"
        cp $PICPATH "$SNAPPATH.jpg"
        $ROOMBACMD -p $PORT --sensors-raw > "$SNAPPATH-sensors.txt"
    else
        $ROOMBACMD -p $PORT --$CMD
    fi
fi

cat <<EOF
</pre>
</body>
</html>
EOF
