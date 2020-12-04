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
// define variables and set to empty values

$hash = "$2y$10$/KaZ.U5S4oJCYvuRsm9dMepZu.5mce85EQGF7O5YZpsXlYHy5.CRi";
//$hash = password_hash("welcome", PASSWORD_DEFAULT);

$submitErr = $passwordErr = $schemaErr = $lognameErr = $personnameErr = $cityErr = $buildingErr = $sensortypeErr = "";
$conn = $password = $schema = $logname = $personname = $sensortype = $city= $building = "";
$passMatch = false;
//phpinfo();

//$sql = "SELECT metric_name, value_type FROM metric JOIN type on metric.t_id = type.type_id WHERE type_name LIKE '".$sensortype."'";

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

//function select_type() {
$sql = "SELECT type_name FROM type";
$types = $conn->query($sql);

$locsql = "SELECT DISTINCT city FROM location";
$cities = $conn->query($locsql);

if ($_SERVER["REQUEST_METHOD"] == "POST") { 
  if (empty($_POST["personname"])) {
    $personnameErr = "Human name is required";
  } else {
    $personname = test_input($_POST["personname"]);
    // check if name is well-formed
    if (!preg_match("/^[a-zA-Z ]*$/",$personname)) {
      $personnameErr = "Only letters and white space allowed";    
    }
    if (strlen($personname) > 254) {
      $personnameErr = "String too long";
    }
  }
    
  if (empty($_POST["logname"])) {
    $lognameErr = "Log name is required";
  } else {
    $logname = test_input($_POST["logname"]);
    // check if logname only contains letters and whitespace
    if (!preg_match("/^[a-zA-Z0-9_-~+.:<> ]*$/",$logname)) {
      $lognameErr = "Invalid characters given"; 
    }
    if (strlen($logname) > 509) {
      $lognameErr = "String too long";
    }
  }

  if (empty($_POST["schema"])) {
    $schemaErr = "";
  } else {
    $schema = test_input($_POST["schema"]);
    // check if schema only contains letters and whitespace
    // if (!preg_match("/^[a-zA-Z0-9_-~+.:<> ]*$/",$schema)) {
    //   $schemaErr = "Invalid characters given"; 
    // }
    if (strlen($schema) > 250) {
      $schemaErr = "String too long";
    }
  }

  if (empty($_POST["sensortype"])) {
    $sensortypeErr = "Sensor type is required";
  } else {
    $sensortype = test_input($_POST["sensortype"]);
    // check if syntax is valid
    if (!preg_match("/^[a-zA-Z0-9-*_~.,' ]*$/",$sensortype)) {
      $sensortypeErr = "Invalid characters given"; 
    }
    if (strlen($sensortype) > 254) {
      $sensortypeErr = "String too long";
    }
  }

  if (empty($_POST["city"])) {
    $cityErr = "";
  } else {
    $city = test_input($_POST["city"]);
    // check if syntax is valid
    if (!preg_match("/^[a-zA-Z0-9-.,;' ]*$/",$city)) {
      $cityErr = "Invalid characters given"; 
    }
  }

  if (empty($_POST["building"])) {
    $buildingErr = "";
  } else {
    $building = test_input($_POST["building"]);
    // check if syntax is valid
    if (!preg_match("/^[a-zA-Z0-9-.,;' ]*$/",$building)) {
      $buildingErr = "Invalid characters given"; 
    }
  }

 if (empty($_POST["password"])) {
    $passwordErr = "Password is required to submit form";
  } else {
    $password = test_input($_POST["password"]);
    // check if password matches
    if (!password_verify($password, $hash)) {
      $passwordErr = "Password doesn't match";
      $passMatch = false;
    } else {
      $passMatch = true;
    }
  }

  if (isset($_POST["submit"])) {
    if (check_errors($personnameErr,$lognameErr,$sensortypeErr)) {
      if ($passMatch) {
        //need to check whether logname already exists in database
        $sql = "SELECT logname FROM sensor WHERE logname = '".$logname."'";
        $result = $conn->query($sql);
        if (!$result) {
            echo "Query failed: (" . $conn->errno . ") " . $conn->error;
        } else {
          if (mysqli_num_rows($result) > 0) {
            //log of this name exists already
            $submitErr = "Cannot submit form. This log name is already in the database.";
          } else {
            //insert the data
            $t_id = "";
            //if new type:
            if (mysqli_num_rows($conn->query("SELECT * FROM type WHERE type_name LIKE '".$sensortype."'")) == 0) {
             $typeinsert = "INSERT INTO type (type_name) VALUES ('".$sensortype."')";
              if ($conn->query($typeinsert)) {
                $submitErr = "New type record created. ";
                $t_id = $conn->insert_id;	 
                /*
                foreach ($keys as $x => $x_value) {
                  $metricinsert = "INSERT INTO metric (metric_name, value_type, t_id) VALUES ('".$x."', '".$x_value."', '".$t_id."')";
                  if ($conn->query($metricinsert)) {
                  } else {
                    $submitErr = $submitErr . "Failed to insert new metric type for " . $x;
                  }
                }
                */
              } else {
                $submitErr = $submitErr . "Failed to insert new sensor type " . $sensortype;
              }

           } else {
              //get the id for known sensortype
              $typequery = "SELECT type_id FROM type WHERE type_name LIKE '".$sensortype."'";
              $result = $conn->query($typequery);
              if ($result) {
                if ($result->num_rows == 1) {
                  while($row = $result->fetch_assoc()) {
                    $t_id = $row["type_id"];
                  }
                }
              }
            }
            $submitErr = $submitErr . " Type id " . $t_id . ". ";
            //if type exists:
            if ($t_id) {
              $sensorinsert = "INSERT INTO sensor (logname, personname, location, t_id) VALUES ('".$logname."', '".$personname."', '".$location."', '".$t_id."')";
              if ($conn->query($sensorinsert)) {
                $submitErr = $submitErr . "Sensor entered into database successfully.";
              } else {
                $submitErr = $submitErr . "Failed to insert new sensor " . $logname;
              }
            }
          }
        }
      }
    } 
    if ($submitErr == "") {
      $submitErr = "Please fix remaining errors.";
    }
  }
}

function check_errors($personnameErr,$lognameErr,$sensortypeErr) { 
  if ($personnameErr == "" and $lognameErr == "" and $sensortypeErr == "") { 
    return true;
  }
  return false;
}

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}
?>
<style>
select { font-size: 14px;
}
input { font-size: 14px;
 }
</style>
<h2>GDP Sensor Registration Form</h2>
<p><span class="error">* required field.</span></p>
<form method="post" action="<?php 
echo htmlspecialchars($_SERVER["PHP_SELF"]);?>">  
  Your name (first and last): <input type="text" name="personname" value="<?php echo $personname;?>">
  <span class="error">* <?php echo $personnameErr;?></span>
  <br><br>

  Logname: <input style="width: 25%;" type="text" name="logname" value="<?php echo $logname;?>">
  <span class="error">* <?php echo $lognameErr;?></span>
  <br><br>

  City: <input list="cities" name="city" value="<?php echo $city;?>">
    <datalist id="cities">
    <?php 
    while ($row = $cities->fetch_assoc()){
        echo "<option value=" . $row['city'] . ">" . $row['city'] . "</option>";
        $buildings[] = $row['city'];
    } 
    ?></datalist>

  Building: <input list="buildings" name="building" value="<?php echo $building;?>">

    <datalist id="buildings">
    <?php 
    echo "<option value=\"jacobs\">". $city . "</option>";
    $sql = "SELECT * FROM location WHERE city = '".$city."'";
    $result = $conn->query($sql);
    if (!$result) {
        echo "<option value=\"error\">". "error" . "</option>";
    } else {
      while ($row = $result->fetch_assoc()){
          echo "<option value=\"success\">". "success" . "</option>";
          echo "<option value=" . $row['building'] . ">" . $row['building'] . "</option>";
      } 
    }
    ?>
    </datalist>

  Room/Area: <input list="rooms" name="room" value="<?php echo $room;?>">

    <datalist id="rooms">
    <?php //echo "<option value=\"jacobs\">". $city . "</option>";
    $sql = "SELECT room FROM room JOIN location ON room.l_id = location.l_id WHERE location.building LIKE '".$building."'";
    $result = $conn->query($sql);
    if (!$result) {
        echo "Query failed: (" . $conn->errno . ") " . $conn->error;
    } else {
      while ($row = $result->fetch_assoc()){
          echo "<option value=" . $row['building'] . ">" . $row['building'] . "</option>";
      } 
    }
    ?>
    </datalist>

  <span class="error"><?php echo " " . $buildingErr;?></span>
<br><br>

  Sensor Type: <input list="typos" name="sensortype" value="<?php echo $sensortype;?>">
    <datalist id="typos">
    <?php 
    while ($row = $types->fetch_assoc()){
        echo "<option value=" . $row['type_name'] . ">" . $row['type_name'] . "</option>";
    } 
    ?></datalist>
  <span class="error">* <?php echo $sensortypeErr;?></span>
  <br><br>

  Data Schema (Eddystone or other URL): <input style="width: 25%;" type="text" name="schema" value="<?php echo $schema;?>">
  <span class="error"><?php echo $schemaErr;?></span>
  <br><br>

  Password: <input type="password" name="password">
  <span class="error">* <?php echo $passwordErr;?></span>
  <br><br>
<input type="submit" name="submit" value="Submit"><span class="error"><?php echo " " . $submitErr;?></span>   
</form>

<?php
  echo "<h2>Your Input:</h2>";
  echo $personname;
  echo "<br>";
  echo $location;
  echo "<br>";
  echo $logname;
  echo "<br>";
  echo $sensortype;
  echo "<br>";
  echo $city . " ";
  echo $building;

?>

</body>
</html>
