<?php
require_once("../php_stat/php_stat_common_define.php");
//require_once('php_common_define.php');


Global $user;
Global $password;
Global $db_name;
Global $host;

$conn = mysql_connect($host, $user, $password);
if (!$conn)
{
	die('Could not connnect: ' . mysql_error());
}

$result = mysql_query("set names utf8;");

$query_str = "select id,level from $db_name.t_horse_info;";

$result = mysql_query($query_str);

$horse_arr = array();

if ($result)
{
	while ($row = mysql_fetch_array($result))
	{

		$id = $row[0];
		$level = $row[1];

		$horse_arr[$id] = $level;
	} 
}
else
{
	echo "$query_str".mysql_error();
	mysql_close($conn);
	exit;
}

foreach($horse_arr as $id=>$level)
{
	echo $id."	".$level."\n";

	if (1 <= $level && $level<= 20)
	{
		$power = $level * 3;
		$intelligence = $level * 3;
		$agility = $level * 2;
		$physique = $level * 3;
	}
	else if (21 <= $level && $level<= 40)
	{
		$power = 20 * 3;
		$intelligence = 20 * 3;
		$agility = 20 * 2;
		$physique = 20 * 3;


		$power += ($level - 20) * 4;
		$intelligence += ($level - 20) * 4;
		$agility += ($level - 20) * 3;
		$physique += ($level - 20) * 4;
	}
	else
	{
		$power = 20 * 3;
		$intelligence = 20 * 3;
		$agility = 20 * 2;
		$physique = 20 * 3;

		$power += 20 * 4;
		$intelligence += 20 * 4;
		$agility += 20 * 3;
		$physique += 20 * 4;

		$power += ($level - 40) * 5;
		$intelligence += ($level - 40) * 5;
		$agility += ($level - 40) * 4;
		$physique += ($level - 40) * 5;
	}

	$query_str = "update $db_name.t_horse_info set power=add_power+$power,intelligence=add_intelligence+$intelligence,agility=add_agility+$agility,physique=add_physique+$physique where id=$id;";

	$result = mysql_query($query_str);

	if (!$result)
	{
		echo "$query_str".mysql_error();
	}
}

mysql_close($conn);
?>
