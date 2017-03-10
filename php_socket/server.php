<?php
// Server 
// 设置错误处理
error_reporting( E_ALL );
// 设置运行时间
set_time_limit( 0 );
// 启用缓冲
ob_implicit_flush();

$ip = "127.0.0.1";
$port = 10258;

$socket = socket_create( AF_INET, SOCK_STREAM, SOL_TCP );
if ( $socket )
	echo "socket_create successed! \n";
else 
	echo "socket_create failed: " . socket_strerror( $socket ) . "\n";

$bind = socket_bind( $socket, $ip, $port );
if ( $bind )
	echo "socket_bind successed. \n";
else
	echo "socket_bind failed: " . socket_strerror( $bind ) . "\n";

$listen = socket_listen( $socket );
if ( $listen )
	echo "socket_listen successed. \n";
else
	echo "socket_listen failed: " . socket_strerror( $listen ) . "\n";

while( true )
{
	$msg = socket_accept( $socket );
	if ( !$msg )
	{
		echo "socket_accept failed. " . socket_strerror( $msg ) . "\n";
		break;
	}
	$welcome = "Welcome to PHP Server !\n";
	socket_write( $msg, $welcome, strlen( $welcome ) );
	while( true )
	{
		$command = strtoupper( trim( socket_read( $msg, 1024 ) ) );
		if ( !$command )
		{
			case "HELLO":
				$write = "hello everybody!";
				break;
			case "QUIT":
				$write = "Bye-Bye.";
				break;
			case "HELP":
				$write = "Hello Quit HELP";
				break;
			default:
				$write = "Error Command.";
				break;
		}
		socket_write( $msg, $write, strlen( $write ) );
		if ( $command == "Quit" )
			break;
	}
	socket_close( $nsg );
}

socket_close( $socket );

?>



