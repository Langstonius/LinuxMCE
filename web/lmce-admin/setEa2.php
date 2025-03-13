<?php
/*
 setEa - temp utility to set the ea for a new Qorbiter
 */
ini_set("display_errors", "off");
$deviceID = -1;
$label = "QOrbiter " . $deviceID;
//initialization area
if (isset($_GET["d"])) {
	$deviceID = $_GET['d'];
	if ($deviceID == "") {
		echo "No Device set";
		die("No Device Set");
	}
} else {
	die("Please specify the device ID with d=xxxx");

}

// Connect to database
$server = "localhost";
$mysqlUser = "root";
$mysqlPass = "";
$conn = mysqli_connect($server, $mysqlUser, $mysqlPass, "pluto_main");
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}

if (isset($_GET['label'])) {
	$label = mysqli_real_escape_string($conn, $_GET['label']);
} else {
	die("No description");
}

/*
 Need to get children devices and do the same as the parent.
 */
echo "Starting for deviceID::" . $deviceID . "<br>";
$deviceName = mysqli_real_escape_string($conn, $_GET['label']);
$mediaPlayerID = $deviceID + 1;
$mobileRoom = -1;
$mobileEa = -1;

if ($conn) {
	$installSql = "SELECT * FROM `Installation` ";
	$iRes = mysqli_query($conn, $installSql) or die(mysqli_error($conn));
	$inst = "";

	while ($row = mysqli_fetch_array($iRes, MYSQLI_ASSOC)) {
		$inst = $row['PK_Installation'];
	}

	$installation = $inst;
	echo "Installation is " . $installation . "<br>Device: " . $label . " <br> ID::" . $deviceID . "<br>";

	if ($installation == "") {
		die("Cant Find Installation");
	}
	//check for duplicate ea, and obtain existing
	if(checkIfDupe($conn)){
		echo "checked duplicates<br>";
	}

	if (setParentDescription($conn)) {
		echo "<br>Set Device Parent Name/Description<br>";
	}

	if (setupMobileRoom($conn)) {
		echo "Mobile Room Setup complete<br>";
	}

	if (fixIntEa($conn)) {
		echo "EntertainArea set <br>";
	}

	if (fixRoomEa($conn)) {
		echo "Checked EAs <br>";
	}

	if (precheckDeviceEntertainArea($conn)) {
		echo "checked device_entArea<br>";
	}

	echo "finished";
}

function checkIfDupe($conn) {
	global $deviceID;
	global $mediaPlayerID;
	
	$GLOBALS['deviceName'] = mysqli_real_escape_string($conn, $_GET['label']);
	$deviceName = $GLOBALS['deviceName'];
	

    echo "checking dupe for $deviceName<br>";
	$sql = "SELECT * from Device_EntertainArea where FK_Device= $mediaPlayerID ;";
	echo "$sql<br>";
	$result = mysqli_query($conn, $sql);
	$existingEa = NULL;
	if (mysqli_num_rows($result) == 0) {
		echo "no device found <br>";
		return true;
	} else {
		echo "Found device matching criteria <br>";
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			$existingEa = $row['FK_EntertainArea'];
			echo "Existing EntertainArea::$existingEa<br>";
		}

		if ($existingEa == NULL) {
			die("Logical Error in finding existing ea in device_entertainarea");
		}

		$query = "SELECT * from EntertainArea WHERE PK_EntertainArea=$existingEa";
		$result = mysqli_query($conn, $query);

		if (mysqli_num_rows($result) == 0) {
			die("Missing Proper EA all together!");
		} else {
			while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
				
				if($row['Description'] == $deviceName){
					echo "nothing wrong with duplicate check<br> ";
					return true;
				} else {
					if($deviceName=="localhost"){
						
						echo "<h2>Detected 'localhost' for device name, leaving existing device name ".$row['Description']." in place</h2><br>";
					
					$GLOBALS['deviceName']=$row['Description'];
					} else {
						
					$GLOBALS['deviceName']=$row['Description'];
						echo "Device name is being updated to  ".$deviceName;
						return true; 
					}
				}	
			}
		}

	}

}

function setParentDescription($conn) {
	global $deviceID;
	$deviceName = $GLOBALS['deviceName'];
	
    echo "Setting Parent device to ".$GLOBALS['deviceName'];


	if ($deviceName == "QOrbiter-Generic" || $deviceName=="localhost"){
			return true;
	}
	

	echo "Setting parent device description label to " . $GLOBALS['deviceName']."<br>";
	$sql = "UPDATE Device SET Description='" . $GLOBALS['deviceName'] . "' WHERE PK_Device='" . $deviceID . "';";
	$result = mysqli_query($conn, $sql);
	return true;
}

function setEntertainArea($conn) {
	global $mediaPlayerID;
	global $mobileRoom;
	$deviceN = $GLOBALS['deviceName'];
	
	$d = mysqli_real_escape_string($conn, $_GET['label']);
	echo "Creating EA " . $GLOBALS['deviceName'] . " in room: " . $mobileRoom . "<br>";

	$sql = "INSERT INTO EntertainArea (FK_Room, Only1Stream,Description,Private,FK_FloorplanObjectType) Values ($mobileRoom,0,'$deviceN',0,52);";
	echo "<br>$sql<br>";
	mysqli_query($conn, $sql);
	$id = mysqli_insert_id($conn);
	echo "last insert id $id<br>";

	$ea = $id;

	if (is_null($ea)) {
		echo "invalid EA ";
		die("cannot continue");
	} else {
		echo "valid ea $ea <br>";
	}

	$sql2 = "SELECT * FROM `Device_EntertainArea` WHERE `FK_Device` =" . $mediaPlayerID . " LIMIT 0, 30 ";
	$result2 = mysqli_query($conn, $sql2) or die(mysqli_error($conn));
	$row = mysqli_fetch_array($result2, MYSQLI_ASSOC);

	if (mysqli_num_rows($result2) == 0) {
		echo "Device not present in Device_EntertainArea, adding to table";
		$sql3 = "INSERT INTO `pluto_main`.`Device_EntertainArea` (`FK_Device`, `FK_EntertainArea`, `psc_id`, `psc_batch`, `psc_user`, `psc_frozen`, `psc_mod`, `psc_restrict`) VALUES ( $mediaPlayerID ,  $ea , NULL, NULL, NULL, '0', CURRENT_TIMESTAMP, NULL);";
		echo "<br>$sql3<br>";
		$result3 = mysqli_query($conn, $sql3) or die(mysqli_error($conn));
	}
}

function setupMobileRoom($conn) {
	global $mobileRoom;
	global $installation;
	
	echo "<br>Connection Found, Starting<br>";

	$roomSql = "SELECT * FROM `Room` WHERE `Description` like 'Mobile' LIMIT 0, 30 ";
	echo "Checking for existing mobile orbiter room <br>";
	$result = mysqli_query($conn, $roomSql) or die(mysqli_error($conn));
	$cnt = mysqli_num_rows($result);

	if ($cnt === 0) {

		$iRoomSql = "INSERT INTO `pluto_main`.`Room` (`PK_Room`, `FK_Installation`, `FK_RoomType`, `Description`, `FK_Icon`, `ManuallyConfigureEA`, `HideFromOrbiter`, `FK_FloorplanObjectType`, `FloorplanInfo`, `psc_id`, `psc_batch`, `psc_user`, `psc_frozen`, `psc_mod`, `psc_restrict`) VALUES (NULL, " . $installation . ", '9', 'Mobile', NULL, '1', '1', NULL, NULL, NULL, NULL, NULL, '0', CURRENT_TIMESTAMP, NULL);";
		$result2 = mysqli_query($conn, $iRoomSql) or die(mysqli_error($conn));
		$lastId = mysqli_insert_id($conn);

		$chkSql = "SELECT * FROM `Room` WHERE `PK_Room` = " . $lastId;
		$result3 = mysqli_query($conn, $chkSql) or die(mysqli_error($conn));
		while ($row = mysqli_fetch_array($result3, MYSQLI_ASSOC)) {
			if ($row['PK_Room']) {
				$mobileRoom = $row['PK_Room'];
				echo "Mobile QOrbiters Room is " . $mobileRoom;
			}
		}
		echo "Not Found, so we've added it. Setting up entertain area now<br>";
	} else {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			if ($row['PK_Room']) {
				$mobileRoom = $row['PK_Room'];
				echo "Mobile QOrbiters Room is " . $mobileRoom . "<br>";
			}
		}

	}

	return true;
}

function fixIntEa($conn) {
	global $mobileRoom;
	global $deviceName;

	$mp = $_GET['d'];
	$mediaPlayerID = $mp + 1;

	echo "<br><b>Checking for incorrect int ea settings for device id::" . $mp . "</b> in room " . $mobileRoom . "<br>";
	//first check that we dont have duplicate entries in the entertain area table
	$sql = "SELECT * FROM `EntertainArea` WHERE `Description` LIKE '" . $mp . "' LIMIT 0, 30 ";
	$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));

	if (mysqli_num_rows($result) == 0) {
		echo "No int EntertainArea for " . $GLOBALS['deviceName'] . " to fix<br>";
		return true;
	} else {
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			echo "Found int EntertainArea. Incorrect setting  for device " . $deviceName . " in " . $row['PK_EntertainArea'] . "<br>";
			$correctionSql = "UPDATE EntertainArea SET Description='" . $GLOBALS['deviceName'] . "', FK_Room='" . $mobileRoom . "'  WHERE PK_EntertainArea = '" . $row['PK_EntertainArea'] . "'";
			$correctionResult = mysqli_query($conn, $correctionSql);
			echo "Setting Corrected.<br>";
			return true;
		}
	}

}

function fixRoomEa($conn) {
	global $mobileRoom;
	$mp = $_GET['d'];
	$mediaPlayerID = $mp + 1;
	echo "<br>Checking named EntertainAreas for correct Settings for device " . $GLOBALS['deviceName'] . "<br>";

	$sql = "SELECT * FROM `EntertainArea` WHERE `Description` LIKE '" . $GLOBALS['deviceName'] . "' LIMIT 0, 30 ";
	$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));

	if (mysqli_num_rows($result) == 0) {
		echo "No EntertainArea present, will create one for device " . $GLOBALS['deviceName'] . "<br>";
		setEntertainArea($conn);
		return true;
	} else {
		echo "Existing EntertainArea found, will validate ...";
		while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
			echo " . ";
			$correctionSql = "UPDATE EntertainArea SET Description = '" . $GLOBALS['deviceName'] . "' , FK_Room=" . $mobileRoom . "  WHERE PK_EntertainArea = " . $row["PK_EntertainArea"] . ";";
			$correctionResult = mysqli_query($conn, $correctionSql);
		}
		echo " complete <br>";
		return true;
	}
}

function precheckDeviceEntertainArea($conn) {
	global $deviceName;
	$mp = $_GET['d'];
	$mediaPlayerID = $mp + 1;

	echo "<br>Checking Device_EntertainArea to validate qMediaPlayer Settings for QOrbiter on device::" . $GLOBALS['deviceName'] . "<br>";

	$sql2 = "SELECT * FROM `Device_EntertainArea` WHERE `FK_Device` =" . $mp . " LIMIT 0, 30 ";
	$result2 = mysqli_query($conn, $sql2) or die(mysqli_error($conn));

	if (mysqli_num_rows($result2) == 0) {
		echo "parent device not set, continuing.<br>";
		$sql = "SELECT * FROM `Device_EntertainArea` WHERE `FK_Device` =" . $mediaPlayerID . " LIMIT 0, 30 ";
		$result = mysqli_query($conn, $sql) or die(mysqli_error($conn));

		if (mysqli_num_rows($result) == 0) {
			echo "Missing Entry, needs to be added for " . $GLOBALS['deviceName'] . "<br> ";

			$check = "SELECT * FROM EntertainArea WHERE Description LIKE '" . $_GET['label'] . "' ";
			$checkResult = mysqli_query($conn, $check) or die(mysqli_error($conn));
			$ea = null;

			echo mysqli_num_rows($checkResult);

			while ($checkRow = mysqli_fetch_array($checkResult, MYSQLI_ASSOC)) {
				$ea = $checkRow['PK_EntertainArea'];
			}

			if (is_null($ea)) {
				echo "invalid EA <br>";
				return true;
			} else {
				echo "EA::" . $ea . "<br>";
			}

			$sql3 = "INSERT INTO `pluto_main`.`Device_EntertainArea` (`FK_Device`, `FK_EntertainArea`, `psc_id`, `psc_batch`, `psc_user`, `psc_frozen`, `psc_mod`, `psc_restrict`) VALUES (" . $mediaPlayerID . ", " . $ea . ", NULL, NULL, NULL, '0', CURRENT_TIMESTAMP, NULL);";
			$result3 = mysqli_query($conn, $sql3) or die(mysqli_error($conn));

			return true;
		} else {
			echo "device in table<br>";
			while ($row = mysqli_fetch_array($result, MYSQLI_ASSOC)) {
				$cleanupSql = "UPDATE Device_EntertainArea SET FK_Device= " . $mediaPlayerID . " WHERE PK_EntertainArea = " . $row["PK_EntertainArea"] . ";";
				$result = mysqli_query($conn, $cleanupSql);
				return true;
			}
		}

	}

	return true;

}

function updateEntertainArea($conn, $device, $location) {
	echo "Updating device to existing EA <br>";

	$mp = $_GET['d'];
	$mediaPlayerID = $mp + 1;
	global $mobileEa;

	if ($mobileEa == -1) {
		echo "invalid ea, exiting updateEntertainArea()";
		return;
	}
	echo "updating...<br>";
	$updatesql = "UPDATE `pluto_main`.`Device_EntertainArea` SET `FK_EntertainArea` = " . $mobileEa . " WHERE `Device_EntertainArea`.`FK_Device` = " . $mediaPlayerID;
	$result = mysqli_query($conn, $updatesql) or die(mysqli_error($conn));
	echo "updated. <br>";
}
  ?>