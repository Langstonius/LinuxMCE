<?php
include_once "config.inc";
include_once '../include/adodb/adodb.inc.php';
include_once '../include/adodb/adodb-errorhandler.inc.php';

// Create ADOdb connection
$db = ADONewConnection('mysqli');
if (!$db->PConnect($DB_SERVER, $DB_LOGIN, $DB_PASSWORD, $DB)) {
    echo("Internal error. Failed to connect to database");
    exit();
}

$sql = "SELECT A.PK_Contact, A.Name, A.Company, A.JobDescription, A.Title, Email, 
        B.CountryCode, B.AreaCode, B.PhoneNumber, B.Extension, C.Description 
        FROM Contact AS A, PhoneNumber AS B, PhoneType AS C 
        WHERE A.PK_Contact=B.FK_Contact AND B.FK_PhoneType=C.PK_PhoneType AND A.EntryID IS NULL 
        ORDER BY A.PK_Contact";

$result = $db->Execute($sql);
if (!$result) {
    echo("Database query error: " . $db->ErrorMsg());
    exit();
}

while ($row = $result->FetchRow()) {
    $fldstring = "";
    foreach ($row as $field) {
        $fldstring .= $field . "~";
    }
    echo $fldstring . "|";
}

if ($result->RecordCount() == 0) {
    echo "End";
}
?>