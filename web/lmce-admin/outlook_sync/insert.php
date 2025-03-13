<?php
include_once "config.inc";
include_once '../include/adodb/adodb.inc.php';
include_once '../include/adodb/adodb-errorhandler.inc.php';

// Create ADOdb connection
$db = ADONewConnection('mysqli');
if (!$db->PConnect($DB_SERVER, $DB_LOGIN, $DB_PASSWORD, $DB)) {
    echo("Error : Internal error. Failed to connect to database|");
    exit();
}

function insertphones($fd, $cid, $db) {
    while ((trim($buffer = fgets($fd, 4096)) != "") && (!feof($fd)) && ($cid != "")) {
        $sql = "SELECT PK_PhoneType FROM PhoneType WHERE UPPER(Description) = ?";
        $result = $db->Execute($sql, array(strtoupper(trim($buffer))));
        
        if ($result && $result->RecordCount() > 0) {
            $row = $result->FetchRow();
            $phonetype = $row[0];
            $buffer = fgets($fd, 4096);
            
            $sql = "INSERT INTO PhoneNumber(FK_Contact, FK_PhoneType, Countrycode, AreaCode, PhoneNumber, Extension) 
                    VALUES (?, ?, " . $buffer . ")";
            $db->Execute($sql, array($cid, $phonetype));
        } else {
            echo "Error : No phone type found " . $sql . "|";
        }
    }
}

$i = 0;
$userfiletemp = $_FILES['fileinsert']['tmp_name'];    
$fd = fopen($userfiletemp, "r");
$e = 0;
$st = "";

if (!feof($fd)) $buffer = fgets($fd, 4096);
if (!feof($fd))
while (!feof($fd)) {
    if ($i == 0) {
        $i++;
        if (trim($buffer) != "xcevw12e9f5kj") {
            fclose($fd);
            exit();
        }
    }
    
    while((($buffer = fgets($fd, 4096))) && (!feof($fd))) {
        if (trim($buffer) != '') break;
    }
    
    if (!(feof($fd))) {
        // Use ADOdb's qstr to safely escape strings
        $buffer = $db->qstr($buffer, false);
        $buffer = substr($buffer, 1, -1); // Remove the quotes added by qstr
        $arrfields = explode("~", $buffer);
        
        $sql = "SELECT PK_Contact FROM Contact WHERE EntryID = ?";
        $rowset = $db->Execute($sql, array($arrfields[0]));
        
        if ($rowset && $rowset->RecordCount() > 0) {
            // update
            $row = $rowset->FetchRow();
            $cid = $row['PK_Contact'];
            
            $sql = "UPDATE Contact SET 
                    Name = ?,
                    Company = ?,
                    JobDescription = ?,
                    Title = ?,
                    Email = ? 
                    WHERE EntryID = ?";
            
            $db->Execute($sql, array(
                $arrfields[1],
                $arrfields[2],
                $arrfields[3],
                $arrfields[4],
                $arrfields[5],
                $arrfields[0]
            ));
            
            $db->Execute("DELETE FROM PhoneNumber WHERE FK_Contact = ?", array($cid));
            insertphones($fd, $cid, $db);
            
        } else {        
            // insert    
            $sql = "INSERT INTO Contact(EntryID, Name, Company, JobDescription, Title, Email) 
                    VALUES (?, ?, ?, ?, ?, ?)";
            
            $result = $db->Execute($sql, array(
                $arrfields[0],
                $arrfields[1],
                $arrfields[2],
                $arrfields[3],
                $arrfields[4],
                $arrfields[5]
            ));
            
            $cid = $db->Insert_ID();
            
            if(!$result) { 
                $e++;
                echo "Error ID: " . $i . "  Execution Query " . $sql . ". Error = " . $db->ErrorMsg() . "|";
                $cid = "";
            }
            
            insertphones($fd, $cid, $db);
        }
    }
}

fclose($fd);
?>