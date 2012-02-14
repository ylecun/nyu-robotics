<?php
// config: edit these two lines to point to your Roomba
$roomba_host = "192.168.0.1";
$roomba_port = 10001;

$cmd = isset($_GET['cmd']) ? $_GET['cmd'] : '';
$vel = isset($_GET['vel']) ? $_GET['vel'] : 200;

function roomba_stop() {
    roomba_drive( 0, 0 );
}
function roomba_go_forward($velocity) {
    roomba_drive( $velocity, 0x8000 );
}
function roomba_go_backward($velocity) {
    roomba_drive( -$velocity, 0x8000 );
}
function roomba_spin_left($velocity) {
    roomba_drive( $velocity, -1 );
}
function roomba_spin_right($velocity) {
    roomba_drive( $velocity,  1 );
}
function roomba_drive($velocity,$radius) {
    $vhi = $velocity >> 8;
    $vlo = $velocity & 0xff;
    $rhi = $radius   >> 8;
    $rlo = $radius   & 0xff;
    print "vhi:$vhi, vlo:$vlo, rhi:$rhi, rlo:$rlo\n";
    roomba_send_cmd( pack("C*", 137, $vhi,$vlo, $rhi,$rlo) );
}
function roomba_init() {
    roomba_send_cmd( pack("C", 128) );  // Start
    usleep(100000);  # wait 100 ms
    roomba_send_cmd( pack("C", 130) );  // CONTROL
    usleep(100000);  # wait 100 ms
}
function roomba_send_cmd( $cmd ) {
    global $roomba_host, $roomba_port;
    $fp = fsockopen( $roomba_host, $roomba_port, $errno, $errstr, 30);
    if (!$fp) {                // couldn't connect
        echo "$errstr ($errno)\n";
    }
    else {     
        fwrite( $fp, $cmd );
        fclose($fp); 
    }
}
function roomba_read_sensors() { 
    global $roomba_host, $roomba_port, $roomba_sensors;
    $fp = fsockopen( $roomba_host, $roomba_port, $errno, $errstr, 30);
    if (!$fp) {                // couldn't connect
        echo "$errstr ($errno)\n";
    }
    else {     
        fwrite( $fp, pack("C", 142 ) ); // SENSORS
        fflush( $fp );
        $raw = fread( $fp, 26 );
        fclose($fp);
        $sensors = unpack("C*", $raw);
        return $sensors;
    }
}
?>

<html>
<head>
<title> Roomba Command </title>
<style> table,td { text-align: center; } </style>
</head>
<body bgcolor=#ddddff>

<h1> Roomba Command </h1>
<form method=GET>
<table border=0>
<tr><td><input type=submit name="cmd" value="init"></td>
    <td>&nbsp;</td>
    <td><input type=submit name="cmd" value="sensors"></td></tr>
<tr><td>&nbsp;</td>
    <td><input type=submit name="cmd" value="forward"></td>
    <td>&nbsp;</td></tr>
<tr><td><input type=submit name="cmd" value="spinleft"></td>
    <td><input type=submit name="cmd" value="stop"></td>
    <td><input type=submit name="cmd" value="spinright"></td></tr>
<tr><td>&nbsp;</td>
    <td><input type=submit name="cmd" value="backward"></td>
    <td>&nbsp;</td></tr>
<tr><td> velocity: </td>
    <td><input type=text size=5 name="vel" value="<?echo $vel?>"></td>
    <td/>&nbsp;</td></tr>
</table>
</form>
<pre>

<?php

if( $cmd ) print "cmd:$cmd\n";

if( $cmd == 'init' ) {
    roomba_init();
}
else if( $cmd == 'stop' ) {
    roomba_stop();
}
else if( $cmd == 'forward' )  {
    roomba_go_forward( $vel );
}
else if( $cmd == 'backward' ) {
    roomba_go_backward( $vel );
}
else if( $cmd == 'spinleft' ) {
    roomba_spin_left( $vel );
}
else if( $cmd == 'spinright' ) {
    roomba_spin_right( $vel );
}
else if( $cmd == 'sensors' ) {
    $sensors = roomba_read_sensors();
    $c = count($sensors);
    for( $i=1; $i<=$c; $i++ ) {
      printf("sensors[$i]: %x\n", $sensors[$i]);
    }
}
else if( $cmd == "drive" ) {
    $vel = $_GET['velocity'];
    $rad = $_GET['radius'];
    roomba_drive( $vel, $rad );
}

?>

</pre>
</body>
</html>
