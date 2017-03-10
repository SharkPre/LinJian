<?php
// Client
// 设置错误处理 
error_reporting(E_ALL);
// 设置处理时间
set_time_limit(0);
$ip = "127.0.0.1";
$port = 1000;

$socket = socket_create( AF_INET, SOCK_STREAM, SOL_TCP);
if ( $socket )
	echo "socket_create successed. \n";
else
	echo "socket_create failed: " . socket_strerror( $socket ) . "\n";

$conn = socket_connect( $socket, $ip, $port );
if ( $conn )
	echo "socket_connect successed.\n";
else 
	echo "socket_connect failed: ". socket_strerror( $conn ) . "\n";

echo socket_read( $socket, 1024);
$stdin = fopen("php://stdin","r");
while( true )
{
	$command = trim( fgets($stdin, 1024));
	socket_write( $socket, $command, strlen($command));
	$msg = trim( socket_read( $socket, 1024 ));
	echo $msg . "\n";
	if ( $msg == "Bye-Bye" )
		break;
}
fclose( $stdin );
socket_close( $socket );

?>

