<?php 
/* $Id: page.paging.php 3217 2006-11-28 01:40:23Z gregmac $ */
//Copyright (C) 2006 Rob Thomas (xrobau@gmail.com)
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of version 2 of the GNU General Public
//License as published by the Free Software Foundation.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//Both of these are used for switch on config.php
$display = isset($_REQUEST['display'])?$_REQUEST['display']:'paging';
$type = isset($_REQUEST['type'])?$_REQUEST['type']:'tool';

$action = isset($_REQUEST['action'])?$_REQUEST['action']:'';
$force_page = isset($_REQUEST['force_page']) ? $_REQUEST['force_page']:0;
$selection = isset($_REQUEST['selection'])?$_REQUEST['selection']:'';
$pagelist = isset($_REQUEST['pagelist'])?$_REQUEST['pagelist']:'';
$pagenbr = isset($_REQUEST['pagenbr'])?$_REQUEST['pagenbr']:'';
$pagegrp = isset($_REQUEST['pagegrp'])?$_REQUEST['pagegrp']:'';

?>

</div>
<?php
// Check to make sure that the paging database is propogated and
// up to date.
paging_init();

switch ($action) {
	case "add":
		paging_sidebar($selection, $type, $display);
		paging_show(null, $display, $type);
		break;
	case "delete":
		paging_del($selection);
		redirect_standard();
		break;
	case "modify":
		paging_sidebar($selection, $type, $display);
		paging_show($selection, $display, $type);
		break;
	case "submit":
		paging_modify($pagegrp, $pagenbr, $pagelist, $force_page);
		redirect_standard();
		break;
	default:
		paging_sidebar($selection, $type, $display);
		paging_text();
}

function paging_text() {
?>
<p><?php echo _("This module is for specific phones that are capable of Paging or Intercom. This section is for configuring group paging, intercom is configured through <strong>Feature Codes</strong>.<br /><br />The current list of supported phones is GXP-2000 with firmware 1.0.13 or higher, Snom phones with 'recent' firmware, Polycom phones (provisioned to auto answer), Linksys/Sipura phones, and a few various others. Any phone that is always set to auto-answer should also work (such as the console extension if configured).") ?></p>
<?php
}

function paging_show($xtn, $display, $type) {
	if ($xtn) {
		$rows = count(paging_get_devs($xtn))+1;
		if ($rows < 5) 
			$rows = 5;
		if ($rows > 20)
			$rows = 20;
		echo "<p><a href='".$_SERVER['PHP_SELF']."?type=${type}&amp;display=${display}&amp;action=delete";
		echo "&amp;selection=${xtn}'>"._("Delete Group")." $xtn</a></p>";
	} else {
		$rows = 5;
	}
	
	$config = paging_get_pagingconfig($xtn);
	
	$force_page = $config['force_page'];
	
	echo "<form name='page_edit' action='".$_SERVER['PHP_SELF']."' method='post' onsubmit='return page_edit_onsubmit();'>\n";
	echo "<input type='hidden' name='display' value='${display}'>\n";
	echo "<input type='hidden' name='type' value='${type}'>\n";
	echo "<input type='hidden' name='pagegrp' value='{$xtn}'>\n";
	echo "<input type='hidden' name='action' value='submit'>\n";
	echo "<table><tr><td colspan=2><h5>";
	echo ($xtn)?_("Modify Paging Group"):_("Add Paging Group")."</h5></td></tr>\n";  ?>
	<tr><td><a href='#' class='info'><?php echo _("Paging Extension") ?><span>
	<?php echo _("The number users will dial to page this group") ?></span></a></td>
	<td><input size='5' type='text' name='pagenbr' value='<?php echo $xtn ?>'></td>
	</tr>
	<tr><td valign='top'><a href='#' class='info'><?php echo _("extension list:")."<span><br>"._("Select extension(s)to page. If using users and devices mode, this will be the device number to page, potentially confusing if extension numbers and device numbers don't match. Use Ctrl key to select multiple..") ?> 
	<br><br></span></a></td>
	<td valign="top"> 
	
	<select multiple="multiple" name="pagelist[]" id="xtnlist" >
	<?php 
	$selected = paging_get_devs($xtn); 
	if (is_null($selected)) $selected = array();
	foreach (core_devices_list() as $device) {
		echo '<option value="'.$device[0].'" ';
		if (array_search($device[0], $selected) !== false) echo ' selected="selected" ';
		echo '>'.$device[0].' - '.$device[1].'</option>';
	}
	?>
	</select>
		
		<br>
	</td></tr>

	<tr><td><label for="force_page"><a href='#' class='info'><?php echo _("Force if busy") ?><span>
	<?php echo _("If selected, will not check if the device is in use before paging it. This means conversations can be interrupted by a page (depending on how the device handles it). This is useful for \"emergency\" paging groups ") ?></span></a></label></td>
	<td><input type='checkbox' name='force_page' id="force_page" value='1' <?php if ($force_page) { echo 'CHECKED'; } ?>></td>
	
	<tr>
	<td colspan="2"><br><h6><input type="submit" name="Submit" type="button" value="<?php echo _("Submit Changes")?>"></h6></td>
	</tr>
	</table>
<script language="javascript">
<!--
var theForm = document.page_edit;
theForm.pagenbr.focus();

function page_edit_onsubmit() {
	var msgInvalidPageExt = "<?php echo _('Please enter a valid Paging Extension'); ?>";
	var msgInvalidExtList = "<?php echo _('Please select at least one extension'); ?>";

	defaultEmptyOK = false;
	if (!isInteger(theForm.pagenbr.value))
		return warnInvalid(theForm.pagenbr, msgInvalidPageExt);
	
	var selected = 0;
	for (var i=0; i < theForm.xtnlist.options.length; i++) {
		if (theForm.xtnlist.options[i].checked) selected += 1;
	}
	if (selected < 1) {
		return warnInvalid(theForm.xtnlist, msgInvalidExtList);
	}
		
	return true;
}

-->
</script>
	</form>
<?php
}

function paging_sidebar($selection, $type, $display) {
	echo "<div class='rnav'><ul>\n";
	echo "<li><a id='".($selection==''?'current':'std')."' ";
	echo "href='config.php?type=${type}&amp;display=${display}&amp;action=add'>"._("Add Paging Group")."</a></li>"; 
	//get the list of paging groups
	$presults = paging_list();
	if ($presults) {
		foreach ($presults as $grouparr) {
			$group = $grouparr[0];
			echo "<li><a id=\"".($selection==$group ? 'current':'std');
			echo "\" href=\"config.php?type=${type}&amp;display=";
			echo "${display}&amp;selection=${group}&amp;action=modify\">";
			echo _("Page Group")." ${group}</a></li>";
		}
	} 
	echo "</ul></div><div class='content'><h2>"._("Paging and Intercom")."</h2>\n";
}
?>
