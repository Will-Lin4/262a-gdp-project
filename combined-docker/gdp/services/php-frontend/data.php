<!DOCTYPE HTML>  
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.error {color: #FF0000;}
</style>
</head>
<body>  

<?php
$servername = "localhost";
$username = "gdp";
$pass = "swarmlab";
$dbname = "sensor_db";
// Create connection
$conn = new mysqli($servername, $username, $pass, $dbname);

// Check connection
if ($conn->connect_error) {
die("Connection failed: " . $conn->connect_error);
} 

//$sql = "SELECT type_name FROM type";
//$types = $conn->query($sql);
$query = "SELECT s.logname, s.location, s.humanname, t.type_name FROM sensor as s JOIN type as t ON s.t_id = t.type_id";

$result = $conn->query($query) or die('Query failed: ');

while ($row = $result->fetch_assoc()) {

   $metricquery = "SELECT m.metric_name, m.value_type FROM metric as m JOIN type as t ON m.t_id = t.type_id WHERE t.type_name = '".$row["type_name"]."'";
   $metrics = $conn->query($metricquery);
   $row["metrics"] = [];
   while ($mrow = $metrics->fetch_assoc()) {
	$row["metrics"][$mrow[metric_name]] = $mrow[value_type];
   }
   //$row["metrics"] = $metrics;
   echo json_encode($row);
   echo ",<br><br>";
}
?>
</body>
</html>
