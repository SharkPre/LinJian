<?php

/**
https://segmentfault.com/a/1190000003029173
简单的web 动态服务器 
*/

class web_config
{
	const PORT = 9003;
	const WEB_ROOT = "/Users/linjianyu/Documents/html";
}

class server 
{
	private $ip;
	private $port;
	public function __construct( $ip, $port )
	{
		$this->ip = $ip;
		$this->port = $port;
		$this->await();
	}
	
	private function await()
	{
		$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if ( $sock < 0 )
		{
			echo "Error: " . socket_strerror( socket_last_error() ) . "\n";
		}
		$ret = socket_bind( $sock, $this->ip, $this->port );
		if ( !$ret )
		{
			echo "BINID FAILED: " . socket_strerror( socket_last_error() ) . "\n";
			exit;
		}
		echo "OK\n";
		$ret = socket_listen( $sock );
		if ( $ret < 0 )
		{
			echo "Listen Failed: " . socket_strerror(socket_last_error()) . "\n";
		}
		do {
			$new_sock = null;
			try {
				$new_sock = socket_accept( $sock );
			} catch ( Exception $e )
			{
				echo $e->getMessage();
				echo "Accept Failed: " . socket_strerror( socket_last_error() ) . "\n";
			}
			
			try {
				$request_string = socket_read($new_sock, 1024);
				$response = $this->output( $request_string );
				socket_write( $new_sock, $response );
				socket_close( $new_socket );
			} catch ( Exception $e) 
			{
				echo $e->getMessage();
				echo "READ FAILED: " . socket_strerror( socket_last_error() ) . "\n";
			}
		}while( true );
	}
	
	private function output( $request_string )
	{
		$request_array = explode( " ", $request_string );
		if ( count( $request_array ) < 2 )
		{
			return $this->not_found();
		}
		$uri = $request_array[1];
		$filename = web_config::WEB_ROOT . $uri ;
		echo "request: " . $filename . "\n";
		if ( file_exists( $filename ) )
		{
			return $this->add_header( file_get_contents( $filename ) );
		}
		else 
		{
			return $this->not_found();
		}
	}
	
	private function not_found()
	{
		$content = "
			<h1>File Not Found. <h1>
		";
		return "HTTP/1.1 4.4 File Not Found\r\nContent-Type: text/html\r\nContent-Length: " . strlen( $content ) . "\r\n\r\n" . $content ;
	}
	
	private function add_header($string)
	{
		return "HTTP/1.1 200 OK \r\nContent-Type: " .strlen($string) . "\r\nServer: mengkang\r\n\r\n".$string;
	}
}
// 使用ip 端口 
$server = new server(web_config::ip, web_config::PORT);