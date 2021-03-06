<?php

require_once("SIOX.php");

class Stats {

static function get_list($names = array())
{
	global $dbcon;

	$conditions = array();
	foreach ($names as $n) {
		$conditions[] = "childname LIKE '%$n%' ";
	}
	$where = "WHERE ". implode("OR ", $conditions);

	if (empty($names))
		$where = "";

	$sql = "SELECT * FROM topology.get_statistics() $where ORDER BY childname ASC";

	$stmt = $dbcon->prepare($sql);

	if (!$stmt->execute()) {
		print_r($dbcon->errorInfo());
		die("Error getting statistics.");
	}

	$list = array();

	while ($row = $stmt->fetch(PDO::FETCH_OBJ))
		$list[] = $row;

	return $list;
}

static function get_stat_id($name)
{
	global $dbcon;

	$sql = "SELECT childobjectid FROM topology.get_statistics() WHERE childname = :name";
	
	$stmt = $dbcon->prepare($sql);
	$stmt->bindParam(':name', $name);

	if (!$stmt->execute()) {
		print_r($dbcon->errorInfo());
		die("Error getting stat id. Name=$name");
	}

	$row = $stmt->fetch(PDO::FETCH_OBJ);

	return $row->childobjectid;	
}


static function get_cumulative($id, $start, $stop)
{
	global $dbcon;

	$sql = "SELECT sum(cast(value as double precision)) AS sum FROM statistics.stats_full WHERE value IS NOT NULL AND childobjectid = :id AND timestamp BETWEEN $start AND $stop";
	
	$stmt = $dbcon->prepare($sql);
	$stmt->bindParam(':id', $id);

	if (!$stmt->execute()) {
		print_r($dbcon->errorInfo());
		die("Error getting statistic. Id=$id, Start=$start, Stop=$stop");
	}

	$row = $stmt->fetch(PDO::FETCH_OBJ);

	return $row->sum;
}

static function store_data($x, $y, $tmp_dir = '/tmp', $start, $stop)
{
	global $dbcon;

	if ($start > 0 && $stop > 0)
		$where = "AND timestamp BETWEEN $start AND $stop";
	else
		$where = "";

	$sql = "SELECT timestamp, value FROM statistics.stats_full WHERE childobjectid = :id AND value IS NOT NULL $where ORDER BY timestamp ASC";
	
	$stmt = $dbcon->prepare($sql);
	$stmt->bindParam(':id', $y);

	if ($start > 0 && $stop > 0) {
//		$stmt->bindParam(':start', $stop, PDO::PARAM_INT);
//		$stmt->bindParam(':stop',  $start, PDO::PARAM_INT);
	}

	if (!$stmt->execute()) {
		print_r($dbcon->errorInfo());
		die("Error getting statistic. Id=$id");
	}

	$data = array();
	$first_tick = -1;

	while ($row = $stmt->fetch(PDO::FETCH_OBJ)) {
		if ($first_tick == -1)
			$first_tick = $row->$x;
		$data[$row->$x - $first_tick] = $row->value;
	}

	$data_string = self::tabulate_kv_array($data);
	$data_file = tempnam($tmp_dir, "data-");
	file_put_contents($data_file, $data_string);

	return $data_file;
}

static function destroy_data($data_file)
{
	unlink($data_file);
}

static function tabulate_kv_array(&$kv_array)
{
	array_walk($kv_array, create_function('&$value, $key', '$value = "$key\t$value";'));
	return implode("\n", $kv_array);
}

}
?>
