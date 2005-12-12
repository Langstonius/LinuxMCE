<?

/* @var $dbADO ADOConnection */
/* @var $rs ADORecordSet */
$out='<br>';
$dbADO->debug=false;
$action = isset($_REQUEST['action'])?cleanString($_REQUEST['action']):'form';
$section=$_REQUEST['section'];

$parentID=(isset($_REQUEST['parentID']))?$_REQUEST['parentID']:0;

if($action=='form'){
	$out.='
		<base target="_self" />
		<script language="JavaScript" type="text/javascript" src="scripts/rte/richtext.js"></script>
		<script language="JavaScript" type="text/javascript">
		<!--
		function submitForm() {
			//make sure hidden and iframe values are in sync before submitting form
			//to sync only 1 rte, use updateRTE(rte)
			//to sync all rtes, use updateRTEs
		
			updateRTEs();
			
			return true;
		}
		
		//Usage: initRTE(imagesPath, includesPath, cssFile)
		initRTE("scripts/rte/images/", "scripts/rte/", "scripts/rte/");
		//-->	
		</script>
		<form action="right.php" method="post" name="addDocument" onsubmit="return submitForm();">
		<input type="hidden" name="section" value="'.$section.'">
		<input type="hidden" name="action" value="add">
			<table>
				<tr>
					<td><b>Title</b></td>
					<td><input type="text" name="Title" value="'.((isset($_SESSION['docTitle']))?stripslashes($_SESSION['docTitle']):'').'"></td>
				</tr>
				<tr>
					<td><b>Order</b></td>
					<td><input type="text" name="docOrder" value="'.((isset($_SESSION['docOrder']))?stripslashes($_SESSION['docOrder']):'0').'"></td>
				</tr>
				<tr>
					<td><b>Parent ID</b></td>
					<td><input type="text" name="parentID" value="'.((isset($_SESSION['parentID']))?stripslashes($_SESSION['parentID']):$parentID).'"></td>
				</tr>
				<tr>
					<td><input type="submit" name="add" value="Add"></td>
					<td align="right"><input type="button" name="preview" value="Preview" onClick="document.addDocument.action.value=\'preview\';updateRTEs();document.addDocument.submit();"></td>
				</tr>
				<tr>
					<td colspan="2">
				<script>
					writeRichText(\'Contents\', \''.((isset($_SESSION['docContents']))?$cleanCode=str_replace(array("\r","\n"),'',@$_SESSION['docContents']):'').'\', 500, 300, true, false);
					//-->
				</script></td>
				</tr>
				<tr>
					<td><input type="submit" name="add" value="Add"></td>
					<td align="right"><input type="button" name="preview" value="Preview" onClick="document.addDocument.action.value=\'preview\';updateRTEs();document.addDocument.submit();"></td>
				</tr>
			</table>
		</form>
		<em>If you want to include a picture or diagram you should include the link as HTML within the document, and e-mail the graphic to <a href="mailto:support@plutohome.com">support@plutohome.com</a>, with an explanation of what document it belongs to.</em>
		';
	if(isset($_SESSION['docTitle'])){
		unset($_SESSION['docTitle']);
		unset($_SESSION['docContents']);
		unset($_SESSION['docOrder']);
		unset($_SESSION['parentID']);
	}

}elseif($action=='preview'){
	$_SESSION['docTitle']=cleanString($_POST['Title']);
	$_SESSION['docContents']=stripslashes($_POST['Contents']);
	$_SESSION['docOrder']=cleanInteger($_POST['docOrder']);
	$_SESSION['parentID']=cleanInteger($_POST['parentID']);

	$out.='
		<form action="'.$section.'.php" method="post" name="addDocument">
		<input type="hidden" name="action" value="save">
		<input type="hidden" name="Title" value="'.$_SESSION['docTitle'].'">
		<input type="hidden" name="parentID" value="'.$_SESSION['parentID'].'">				
		<input type="hidden" name="docOrder" value="'.$_SESSION['docOrder'].'">
				<table>
					<tr>
						<td><b>'.$_SESSION['parentID'].' - '.$_SESSION['docOrder'].' '.$_SESSION['docTitle'].'</b></td>
					</tr>
					<tr>
						<td>'.$_SESSION['docContents'].'</td>
					</tr>
					<tr>
						<td align="center"><input type="button" name="back" value="Back" onClick="self.location=\'right.php?section=documents/addDocument&action=form\'"> <input type="submit" name="add" value="Add"></td>
					</tr>
				</table>
		</form>		
		';
}
else{
	if(isset($_POST['add'])){
		$contents=(isset($_POST['Contents']))?stripslashes($_POST['Contents']):$_SESSION['docContents'];
		$title=cleanString($_POST['Title']);
		$parentID=cleanInteger($_POST['parentID']);
		$docOrder=cleanInteger($_POST['docOrder']);

		$insertDocument='INSERT INTO Document (Title,Contents,`Order`,FK_Document_Parent) VALUES (?,?,?,?)';
		$dbADO->Execute($insertDocument,array($title,$contents,$docOrder,$parentID));

		$docID=$dbADO->Insert_ID();
		$out='
				<script>
					try{
						// if add document was called from pluto admin, reload opener
						top.opener.location.reload();
					}
					catch(e){
						//
					}
					top.location="index.php?section=document&docID='.$docID.'";
				</script>
			';

	}

}

$output->setBody($out);
$output->setTitle(APPLICATION_NAME);
$output->output();

?>