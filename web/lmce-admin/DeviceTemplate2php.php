<?php
/* 
 * DeviceTemplate2PHP is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * DeviceTemplate2PHP is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA
 *
 *  Copyright 2008 posde@web.de
 *
 * 	v0.01 - 2008-09-23 - Sharm el-Sheikh version
 * 2009-05-18 - Rewrite by Jason Richardson <merkur2k@gmail.com>
 * - Change export data to be a serialized php associative array
 * - Add functions to web-enable the entire shebang
 * 2025-03-12 - Update to use ADOdb instead of direct mysql functions for PHP 7+ compatibility
 */
error_reporting(E_ALL);

// Include database configuration
include_once('include/config/globalconfig.inc.php');
include_once('include/config/database.inc.php');

// The device template to export
$pk_value = @$_REQUEST['DeviceTemplateID'];
// Used to override the warning for exporting a modified stock template
$force_export = @$_REQUEST["forceExport"] || false;
// Setting forceExportAll will force the script to output all related devices even if
// they have not been modified. There is no user interface option for this as it is just for testing.
$force_export_all = @$_REQUEST["forceExportAll"] || false;
// Also used when overriding the warning for exporting a modified stock template.
$force_export_type = @$_REQUEST["forceExportType"];
// import or export...
$mode = @$_REQUEST["mode"];
// Result of confirmation request
$confirm = @$_REQUEST["confirm"];

if (!$mode) {
  print "Please enter the device template ID of a device template to export:
<form method=GET>
<input type=text name=DeviceTemplateID value=$pk_value>
<input type=submit value=Export>
<input type=hidden name=mode value=export>
</form>
<P>OR<P>
Please select a file to import:
<form method=POST enctype='multipart/form-data'>
<input type=file name=importFile>
<input type=submit value=Import>
<input type=hidden name=mode value=import>
</form>
";
  die ();
}

// Use the existing database connection from database.inc.php
global $dbADO;

if ($mode == "export") {
  $pk_value = intval($pk_value);

  $query = "SELECT * FROM DeviceTemplate WHERE PK_DeviceTemplate = ?";
  $result = $dbADO->Execute($query, array($pk_value));
  
  if (!$result || $result->RecordCount() != 1) {
    if (!$result || $result->RecordCount() == 0) {
      print "<p>There is no DeviceTemplate with ID number $pk_value</p>";
    } else {
      print "<p>There is more than one DeviceTemplate with ID number $pk_value. THIS IS WRONG!</p>";
    }
    die ("<p>Export aborted!</p>");
  }
  
  $row = $result->FetchNextObject(false);

  if (!$confirm) {
    if ($row->PSC_MOD == '0000-00-00 00:00:00') {
      // Template came with the original install or from a SQLcvs update and has not been modified
      // No need to do anything with this case
      print "DeviceTemplate ".$row->PK_DEVICETEMPLATE." (".$row->DESCRIPTION.") has not been modified. There are no modifications to export.<P>Export Aborted!<BR>";
      $type = "old";
      $force_export || die ();
    } else if ($row->PSC_ID > 0) {
      // Template came with the original install or from a SQLcvs update and has been modified.
      // Will export.
      print "DeviceTemplate ".$row->PK_DEVICETEMPLATE." (".$row->DESCRIPTION.") is an original template that has been modified.<BR>Replacing a device template from the original install will most likely cause serious problems.
	It is recommended that you export this as a new device.
	<form method=GET>
	<input type=hidden name=DeviceTemplateID value=$pk_value>
	<input type=hidden name=mode value=$mode>
	<input type=radio name=forceExportType value='old'> I know what I am doing, continue export as is.<BR>
	<input type=radio name=forceExportType value='new' checked> Export as a new device.<BR>
	<input type=hidden name=forceExport value=true>
	<input type=hidden name=confirm value=true>
	<input type=submit value='Continue export'>
	</form>";
      $type = "old";
      $force_export || die ();
    } else {
      // Template was added by the user after install.
      // Will export.
      print "DeviceTemplate ".$row->PK_DEVICETEMPLATE." (".$row->DESCRIPTION.") is a custom template.<BR>
	  <form method=GET>
	<input type=hidden name=DeviceTemplateID value=$pk_value>
	<input type=hidden name=mode value=$mode>
	  <input type=hidden name=forceExport value=true>
	<input type=hidden name=confirm value=true>
	<input type=submit value='Continue export'>
	</form>";
      $type = "new";
    }

    // Start the actual export
    print "Exporting DeviceTemplate ".$row->PK_DEVICETEMPLATE." (".$row->DESCRIPTION.").<BR>";
  }
  
  // Create a container object to hold all the data related to this device.
  $device = array();
  $device["id"] = $row->PK_DEVICETEMPLATE;
  $device["type"] = $force_export_type ? $force_export_type : $type;
  $device["data"] = array();
  $device["related"] = array();
  $related = array();

  function CreateInsert(&$device, $pk_value, $table, $keyColumn = "", $newKey = "") {
    global $dbADO;
    
    if ($keyColumn == "") {
      $keyColumn = "PK_" . $table;
    }
    
    // We take all data from the original table.
    $query = "SELECT * FROM $table WHERE $keyColumn = ?";
    $params = array($pk_value);
    
    // Ignore related device entries that have not been modified.
    if ($table != 'DeviceTemplate' && !$GLOBALS["force_export_all"]) {
      $query .= " AND psc_mod > 0";
    }
    
    $result = $dbADO->Execute($query, $params);
    if (!$result) {
      return array();
    }
    
    $externalTables = array();
    
    if ($result->RecordCount() == 0) {
      $externalTables = array("-1", "-1");
    } else {
      // Get the relevant record(s)
      while ($line = $result->FetchRow()) {
        $newrow = array();
        
        $fieldNum = 0;
        foreach ($result->FieldArray() as $field) {
          $fieldName = $field->name;
          $fieldValue = $line[$fieldName];
          $prefix = substr($fieldName, 0, 3);
          
          // Usually we insert the field value
          $insertField = true;
          
          // Except, if the value is NULL, a primary key, or one of the psc_ variables for sqlCVS.
          if (is_null($fieldValue)) {
            $insertField = false;
          }
          if ($prefix == "PK_") {
            $insertField = false;
          }
          if ($prefix == "psc") {
            $insertField = false;
          }
          
          if ($insertField) {
            $value = $fieldValue;
            
            $withoutPrefix = substr($fieldName, 3);
            if ($fieldName == $keyColumn) {
              $value = -1;
            }
            
            // We need to take all the foreign key associated tables with us.
            if (($prefix == "FK_") && ($table == "DeviceTemplate") && $insertField) {
              $returnValue = CreateInsert($device, $fieldValue, $withoutPrefix);
              
              // Only create a special entry, IF we got a result
              $specialEntryRequired = false;
              if (@$returnValue[0] != -1) {
                $specialEntryRequired = true;
              }
              
              // We only add new PK_s for children connections to the DeviceTemplate table.
              if ($table != "DeviceTemplate") {
                $specialEntryRequired = false;
              }
              
              if ($specialEntryRequired) {
                if ($withoutPrefix == "InfraredGroup") {
                  CreateInsert($device, $fieldValue, "InfraredGroup_Command", "FK_InfraredGroup");
                }
              }
            }
            
            $newrow[$fieldName] = $value;
          }
          
          $fieldNum++;
        }
        
        if ($table == 'DeviceTemplate') {
          $device["data"] = $newrow;
        } else {
          $device["related"][$table] = $newrow;
        }
      }
    }
    
    return $externalTables;
  }

  // Create the INSERT code for the maintable and its children.
  CreateInsert($device, $pk_value, "DeviceTemplate");

  // Create all other tables that may contain additional device template information.
  $tables = array(
    "CommandGroup_D_Command",
    "DHCPDevice",
    "DeviceTemplate_AV",
    "DeviceTemplate_DSPMode",
    "DeviceTemplate_DeviceCategory_ControlledVia",
    "DeviceTemplate_DeviceCommandGroup",
    "DeviceTemplate_DeviceData",
    "DeviceTemplate_DeviceTemplate_ControlledVia",
    "DeviceTemplate_DeviceTemplate_Related",
    "DeviceTemplate_Event",
    "DeviceTemplate_InfraredGroup",
    "DeviceTemplate_Input",
    "DeviceTemplate_MediaType",
    "DeviceTemplate_Output",
    "DeviceTemplate_PageSetup",
    "InstallWizard",
    "Screen_DesignObj",
    "StartupScript"
  );

  foreach ($tables as $table) {
    CreateInsert($device, $pk_value, $table, "FK_DeviceTemplate");
  }

  $data = serialize($device);
  header("Content-type: application/octet-stream");
  header("Content-Disposition: attachment; filename=\"device$pk_value.dt\"");
  print $data;
} else if ($mode == "import") {
  $file = fopen($_FILES["importFile"]["tmp_name"], "r");
  $data = fread($file, filesize($_FILES["importFile"]["tmp_name"]));
  fclose($file);
  
  // Now for the insert
  $newdevice = unserialize($data);
  
  if ($newdevice["type"] == "new") {
    print "<P>Will create new template<BR>";
    print "Creating DeviceTemplate entry<BR>";
    
    // Build the parameterized query
    $fields = array_keys($newdevice["data"]);
    $placeholders = array_fill(0, count($fields), '?');
    $values = array_values($newdevice["data"]);
    
    $query = "INSERT INTO DeviceTemplate (" . implode(", ", $fields) . ") VALUES (" . implode(", ", $placeholders) . ")";
    print $query . "<BR>";
    
    $result = $dbADO->Execute($query, $values);
    if (!$result) {
      print $dbADO->ErrorMsg();
      die();
    }
    
    $newID = $dbADO->Insert_ID();
    
    foreach ($newdevice["related"] as $table => $tabledata) {
      // Replace foreign key placeholder with correct value
      foreach ($tabledata as $key => $value) {
        if ($value == -1) {
          $tabledata[$key] = $newID;
        }
      }
      
      print "Creating $table entry<BR>";
      
      // Build the parameterized query
      $fields = array_keys($tabledata);
      $placeholders = array_fill(0, count($fields), '?');
      $values = array_values($tabledata);
      
      $query = "INSERT INTO $table (" . implode(", ", $fields) . ") VALUES (" . implode(", ", $placeholders) . ")";
      print $query . "<BR>";
      
      $result = $dbADO->Execute($query, $values);
      if (!$result) {
        print $dbADO->ErrorMsg();
        die();
      }
    }
  } else if ($newdevice["type"] == "old") {
    print "<P>Will replace old template<BR>";
    print "Replacing DeviceTemplate entry<BR>";
    
    $query = "DELETE FROM DeviceTemplate WHERE PK_DeviceTemplate = ?";
    print $query . "<BR>";
    $result = $dbADO->Execute($query, array($newdevice["id"]));
    
    $newdevice["data"]["PK_DeviceTemplate"] = $newdevice["id"];
    
    // Build the parameterized query
    $fields = array_keys($newdevice["data"]);
    $placeholders = array_fill(0, count($fields), '?');
    $values = array_values($newdevice["data"]);
    
    $query = "INSERT INTO DeviceTemplate (" . implode(", ", $fields) . ") VALUES (" . implode(", ", $placeholders) . ")";
    print $query . "<BR>";
    
    $result = $dbADO->Execute($query, $values);
    if (!$result) {
      print $dbADO->ErrorMsg();
      die();
    }
    
    $newID = $dbADO->Insert_ID();
    
    foreach ($newdevice["related"] as $table => $tabledata) {
      // Replace foreign key placeholder with correct value
      foreach ($tabledata as $key => $value) {
        if ($value == -1) {
          $tabledata[$key] = $newID;
        }
      }
      
      print "Creating $table entry<BR>";
      
      // Build the parameterized query
      $fields = array_keys($tabledata);
      $placeholders = array_fill(0, count($fields), '?');
      $values = array_values($tabledata);
      
      $query = "INSERT INTO $table (" . implode(", ", $fields) . ") VALUES (" . implode(", ", $placeholders) . ")";
      print $query . "<BR>";
      
      $result = $dbADO->Execute($query, $values);
      if (!$result) {
        print $dbADO->ErrorMsg();
        die();
      }
    }
  }
  
  print "Done!";
}
?>
