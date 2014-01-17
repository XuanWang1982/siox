<?php 
require_once("include/DB.php"); 
require_once("include/Activity.php"); 
$site_title = "Activity overview";
$site_description = "Overview of the stored activities";
require_once("header.php"); 
?>

<h1>Activity List</h1>

<?php $activities = Activity::get_list(); ?>

<form name="purge_frm" method="post" action="purge.php">
	<input type="submit" value="Purge all actvities" onclick="return confirm('Dude, you are gonna delete all data. Are you sure?')" />
</form>

<br /><br />

<table cellspacing="0" cellpadding="0">
<thead>
	<tr>
		<th>#</th>
		<th>Function</th>
		<th>Time start</th>
		<th>Time stop</th>
		<th>Duration [µs]</th>
		<th>Error code</th>
	</tr>
</thead>
<tbody>
<?php $i = 0; ?>
<?php foreach ($activities as $a): ?>
	<tr class="<?=$i++ % 2 == 0 ? "even" : "odd";?>" onclick="window.location='activity.php?unique_id=<?=$a->unique_id?>'">
		<td><?=$a->unique_id?></td>
		<td><?=$a->activity_name?></td>
		<td><?=date("d.m.Y H:i:s", floor($a->time_start / 1000000000)).".<b>".($a->time_start % 1000000000)."</b>"?></td>
		<td><?=date("d.m.Y H:i:s", floor($a->time_stop / 1000000000)).".<b>".($a->time_stop % 1000000000)."</b>"?></td>
		<td align="center"><?=round(($a->time_stop - $a->time_start) / 1000, 3)?></td>
		<td><?=$a->error_value?></td>
	</tr>
<?php endforeach ?>
</tbody>
</table>

<?php require_once("footer.php"); ?>