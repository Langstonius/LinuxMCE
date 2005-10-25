<?
function searchDocs($output,$dbADO,$conn){
	/* @var $dbADO ADOConnection */
	/* @var $rs ADORecordSet */

	$searchString=@$_REQUEST['searchString'];
	
	$out='
	<table width="100%" border="0" cellpadding="0" cellspacing="0" class="maintable">
		<tr>
			<td align="left" class="insidetable"><img src="images/titles/search.gif"></td>
		</tr>	
		<tr>
			<td class="insidetable2">';
	if(strpos($searchString,' ')!==false){
		$words=explode(' ',strtolower($searchString));
		$filterByWords='';
		foreach ($words AS $word)
			$filterByWords.=" OR Title LIKE '% $word %' OR Contents LIKE '% $word %'";
	}
	$rs=$dbADO->Execute('SELECT * FROM Document WHERE Title LIKE ? OR Contents LIKE ?'.@$filterByWords,array('%'.$searchString.'%','%'.$searchString.'%'));
	$docsFound=$rs->RecordCount();
	$out.='
		<table>
		<tr>
			<td colspan="2"><span class="title">Searching for: '.$searchString.'</span></td>
		</tr>	
		<tr>
			<td colspan="2"><span class="title">Results found in DOCUMENTS: '.$docsFound.'</span></td>
		</tr>
		<tr>
			<td width="20"></td>
			<td class="insidetable2">';
	// search in documents
	$pos=0;
	while($rowDocs=$rs->FetchRow()){
		$pos++;
		$out.='<B>'.highlight($searchString,$rowDocs['Title']).'</B><br>'.getPartialContent($searchString,$rowDocs['Contents']).'<br>';
		$out.='<div align="right" style="background:#EEEEEE;"><a href="support/index.php?section=document&docID='.$rowDocs['PK_Document'].'">Link</a></div>';
	}		
	$out.='		</td>
	  		</tr>
	  	</table>
		</td>
	</tr>
</table>';
         
	  $output->setScriptCalendar('null');
	  $output->setScriptTRColor('null');
	  $output->setBody($out);
	  $output->setTitle(APPLICATION_NAME."::Search");
	  $output->output();
}

function getPartialContent($target,$str)
{
	preg_match("/\n(.*)$target(.*)\./i",$str,$matches);
	
	if(isset($matches[0])){
		$part=(ereg('support/',$matches[0]))?$matches[0]:highlight($target,$matches[0]);
		return strip_tags($part,'<a>');
	}
	else
		return nl2br(strip_tags($str,'<a>'));
}

function highlight($target,$str)
{
	return str_replace($target,'<font color="blue"><b>'.$target.'</b></font>',$str);
}
?>

