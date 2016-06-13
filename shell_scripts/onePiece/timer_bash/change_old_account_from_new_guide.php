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

function add_task($account_id, $task_id, $progress)
{
Global $db_name;

	$query_str = "insert into $db_name.t_task_info(platform_gamesvr_id, task_id, progress) values('".$account_id."', $task_id, $progress);";

	$result = mysql_query($query_str);

	if ($result)
	{
		echo $account_id." add task ".$task_id." success"."\n";
	}
}




function do_less_than_21_level($uid, $account_id, $level)
{
Global $db_name;

	//---------------------------------------------------------------------------- 
	//提升玩家等级到21级
	$query_str = "update $db_name.t_character_info set level=21 where platform_gamesvr_id='".$account_id."' and char_flag=0;";

	$result = mysql_query($query_str);

	if ($result)
	{
		echo $account_id." change level success"."\n";
	}
	else
	{
		echo "$query_str".mysql_error();
		mysql_close($conn);
		exit;
	}


	//---------------------------------------------------------------------------- 
	//删新手引导
	$query_str = "delete from $db_name.t_task_info where platform_gamesvr_id='".$account_id."' and task_id >=613201;";

	$result = mysql_query($query_str);

	if ($result)
	{
		echo $account_id." delete guide task success"."\n";
	}
	else
	{
		echo "$query_str".mysql_error();
		mysql_close($conn);
		exit;
	}



	add_task($account_id, 340035, 0);
	add_task($account_id, 340036, 0);
	add_task($account_id, 350351, 0);
	add_task($account_id, 613221, 1);
}


function do_saga_bless_task($account_id)
{
	Global $db_name;

	$has_first_charge = 0;

	//---------------------------------------------------------------------------- 
	//是否首充
	$query_str = "select count from $db_name.t_user_event_info where platform_gamesvr_id='".$account_id."' and event_type=400007;";

	$result = mysql_query($query_str);

	if ($result)
	{
		while ($row = mysql_fetch_array($result))
		{

			$has_first_charge = $row[0];
		}
	}
	else
	{
		echo "$query_str".mysql_error();
		mysql_close($conn);
		exit;
	}

	if (!$has_first_charge)
	{
		add_task($account_id, 340284, 0);
		add_task($account_id, 340285, 0);
		add_task($account_id, 340286, 0);
		add_task($account_id, 350331, 0);
		add_task($account_id, 350347, 0);
	}
}

function handle_old_account()
{
Global $db_name;

	$delete_task_list = "(340011,340014,340007,330167,340024,340001,340002,340003,350213,350214,350215,340010,340009,340012,350349, ".
		"350350,340016,340017,340019,340020,350352,350353,340026,340025,350201,330185,340028,340031,340030,340011,340014,340007,330167,340024 ".
		")";

	$query_str = "delete from $db_name.t_task_info where task_id in $delete_task_list;";

	$result = mysql_query($query_str);

	if ($result)
	{
		echo "delete task list success"."\n";
	}
	else
	{
		echo "$query_str".mysql_error();
		mysql_close($conn);
		exit;
	}

	//---------------------------------------------------------------------------- 
	//查出主将等级
	$query_str = "select id,platform_gamesvr_id,level from $db_name.t_character_info where char_flag=0 and id>=10000;";

	$result = mysql_query($query_str);

	if ($result)
	{
		while ($row = mysql_fetch_array($result))
		{

			$uid = $row[0];
			$account_id = $row[1];
			$level = $row[2];
			echo $account_id."	".$level."\n";


			if ($level < 21)
			{
				do_less_than_21_level($uid, $account_id, $level);
			}

			do_saga_bless_task($account_id);
		}
	}
	else
	{
		echo "$query_str".mysql_error();
		mysql_close($conn);
		exit;
	}
}


handle_old_account();

?>
