<?php

/*
	Copyright 2010 - Langston Ball
	golgoj4@gmail.com	
	GPLv2 Licensed
	GPLv2 Licensed
	TVDB Data Grabber
	TvDB.com data grabber for television shows from the web admin, both for single and multiple files 
	
	This file contains functions for batch processing.
	
*/
	require_once(APPROOT.'operations/mediaBrowser/tvdbUtils.php'); 
	
function  tvdbBatch($output,$mediadbADO,$dbADO)	 
{	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/editMediaFile.lang.php');


$scriptInHead='<script src="javascript/prototype.js" type="text/javascript" language="JavaScript"></script>
 			<script src="javascript/scriptaculous.js" type="text/javascript" language="JavaScript"></script>
 			
 		<style>	
 			#container{
			width: 80%;
			margin: 0 auto;
			}
			#containerHeader{
			
			}
			.tableContainer{
			
			}			
	
  </style>';

$out.='<hr>
<form action="" method="post">';

// SCAN FOR TV SHOWS
if ($_POST['action']=='scan')
{
	// Start the scan

		//load attribute names
		$mediaTypeSQL = "SELECT PK_MediaType, Description FROM MediaType WHERE PK_MediaType = 2"; 
		$res1=$mediadbADO->Execute($mediaTypeSQL);
		$rowMEDIATYPE=$res1->FetchRow();
		$TVMediaTypeID = $rowMEDIATYPE['PK_MediaType'];
		$querySeries = "SELECT FileFormat.Description AS FF,FK_FileFormat, FK_MediaType,
		FK_MediaSubType, File.PK_File, Filename, Path, File.DateAdded, File.File_Size, File.Missing 
		FROM File 
		LEFT JOIN FileFormat ON FileFormat.PK_FileFormat=File.FK_FileFormat 
		WHERE File.EK_MediaType=2 AND File.MimeType='video/mpeg'  order by Filename LIMIT 100"; 
		
		$array = array();
		$resSeries=$mediadbADO->Execute($querySeries); //get files from Database, replace with array of values
		$numFilesInDb = $resSeries->RecordCount();

		if($numFilesInDb == 0) //See if files exists in DB
		{
		$out.='<hr><p class="form"><strong>No Results:</strong> Tv Shows to update.  Import files first</p>';
		}
		else
		{
		$out.='<div id="containerHeader">
		<h3>TV Shows in Database: '.$numFilesInDb.' files found</h3>
		</div>	
		<div id="container">';
		$out.='<hr><p><strong>Instructions:</strong> <br>1: Use to Search box to search for your show <br>2: Select Show from the search results<br>3: Choose season and episode<br>3: Press "Batch Submit" to add metadata to database.<br>
		
		<form action="index.php" method="post" name="tvdbsettings">
		<input type="hidden" name="section" value="tvdbbatch">
		<input type="hidden" name="action" value="settings">
		</form>
		
		<form action="index.php" method="post" name="tvdbBatch">
		<input type="hidden" name="section" value="tvdbbatch">
		<input type="hidden" name="action" value="update">
			<table width="100%">
			<tr>
				<td>TVDB.com Series Name:</td><td><input readonly type="text" name="tvdbName" id="tvdbName" value="" size="50"></td>
			</tr>
			<tr>
				<td>TVDB.com SeriesID:</td><td><input readonly type="text" name="seriesID" id="seriesID" value="" size="10">
			</tr>
			<tr>
				<td>Search:</td><td><input type="text" id="seriesSearch" name="seriesSearch" size="50">
				<input type="button" value="Search Series" onclick="getSeries();"></td>
			</tr>
			<tr>
				<td>Language:</td><td>
					<select name="lang">
						<option value="en">English</option>
						<option value="de">Deutsch</option>
						<option value="fr">Français</option>
						<option value="se">Swedish</option>
						<option value="nl">Nederlands</option>
					</select>
				</td>
			</tr>
			<tr>
				<td></td><td><br>
					<input type="button" value="Batch Submit" onclick="javascript:document.tvdbBatch.submit();">
				</td>
			</tr>
			</table>
			<div id="seriesResults"></div>
			<hr>
			<div id="fileResults">
			<table id="files" style="width: 100%;">
			<tr>
				<th>Filename</th>
				<th>Season No</th>
				<th>Episode No</th>
				<th>Resolution</th>
			</tr>';
			
			while($rowSeries=$resSeries->FetchRow())
			{	
			$f= $rowSeries['PK_File'];
			$name = $rowSeries['Filename'];
			$res = $rowSeries['FF'];
			if ($res=='') $res='Unknown';
			$out.= '
			<tr>
				<td>'.$name.' <input type="hidden" name="file[]" id="file_'.$f.'" value="'.$f.'"></td>
				<td><input type="text" name="sNo[]" id="sNo_'.$f.'" value="" size="2"></td>
				<td><input type="text" name="epNo[]" id="epNo_'.$f.'" value="" size="2"></td>
				<td>'.$res.'<input type="hidden" name="rezOver[]" id="rezOver_'.$f.'" value="'.$rowSeries['FK_FileFormat'].'"></td>
			</tr>
			';
			}
			
			$out.='</table></div>';
			
			$out.='</div></form>
						
		<script type="text/javascript">
		function getSeries(){
				var s = document.getElementById("seriesSearch").value;
				new Ajax.Request("operations/mediaBrowser/tvdbSearch.php?s="+s, {
						method:"get",
						onComplete: function(transport){
							document.getElementById("seriesResults").innerHTML = transport.responseText;
					}
				});
		}
		function getShow(sid, sname){
			var n = document.getElementById("tvdbName");
			n.value = sname;
			var id = document.getElementById("seriesID");
			id.value = sid;
		}
		</script>';
		}
	
}
// UPDATE TV SHOWS
elseif ($_POST['action']=='update')
{
		include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
		include(APPROOT.'/languages/'.$GLOBALS['lang'].'/editMediaFile.lang.php');
		$pass=""; $user="root"; $db="pluto_media"; 
		$con = mysqli_connect("localhost", $user, $pass, $db) or die(mysqli_error($con)); 	 // connection 
		if (!$con) { 
			die('Could not Connect' . mysqli_error($con)); //error messaging
			$connMessage="Fail";
		}
		$connMessage="Conn Good"; 	
		
		$batchArray= array();
		$i = 0;
		$updateCount = count($_POST['file']);	
		//add data to folder
		while ($i < $updateCount) {		
		$batchArray[$_POST['file'][$i]] = array('seriesID'=>$_POST['seriesID'], 'episodeNo'=>$_POST['epNo'][$i], 'seasonNo'=>$_POST['sNo'][$i], 'rez'=>$_POST['rezOver'][$i] ); 
		$i++;
		}
			
		$out.='<table width="%85" align="center"><td>';	
		foreach ($batchArray as $key =>$val)
		{
		//check if season and episode exists
		if(strcmp($val['seasonNo'], '')!== 0 && strcmp($val['episodeNo'], '')!== 0)
		{
		$fileIdent = $key;
		
		//check for series ID
		$sIdent = $val['seriesID']; 
		$seaIdent = $val['seasonNo'];
		$epIdent = $val['episodeNo'];
		$rez = $val['rez'];
		
		$out.='<h2>Starting to process TV Show</h2>';
		
		//try to determine file name from FileId
		$query= "SELECT Filename FROM File WHERE PK_File='$fileIdent'";		
		$result = mysqli_query($con, $query) or die(mysqli_error($con));
		$out.='<hr><h3>Filename:</h3>';
		while ($row=mysqli_fetch_array($result, MYSQLI_ASSOC)) {
		$out.=$row['Filename'];
		}
		$out.='<h3>PK_File ID: </h3>'.$fileIdent;
		$result = mysqli_query($con, $query) or die(mysqli_error($con));
		$row=mysqli_fetch_array($result, MYSQLI_ASSOC);
		
		$seriesData=array(); 
		$episodeData=array();
		
		//check file system for tvdbxml
		//download series data to file (only if not exists
		$now = date("Y-m-d", mktime(0, 0, 0, date("m")  , date("d"), date("Y"))); 
		$apiKey = "4C6CEBDFB4558279" ;

		$xmlPath ='/operations/mediaBrowser/tvDBxml';
		if (!is_dir($xmlPath)) {
			$out.='<div>Dir '.$xmlPath.' does not exist, creating...</div>';
			$dirPath = APPROOT.$xmlPath;
			mkdir($dirPath, 0777);
		}
		
		$dlPath = APPROOT.$xmlPath.'/'.$sIdent;
		if (!is_dir($dlPath)) {
			$out.='<div>Dir '.$dlPath.' does not exist, creating...</div>';
			mkdir($dlPath, 0777);
		}
		
		$out.='<h3>TVDB Data Path: </h3>'.$dlPath;
		
		// tvdb data is cached, so check if cache is more than 24hours old
		$cacheTime = 0;
		$cacheFilename = $dlPath."/".$sIdent.".".$_POST['lang'].".xml";
		if (file_exists($cacheFilename)) {
			$cacheTime = time() - filemtime($cacheFilename);
		}
		
		// check time
		if (file_exists($cacheFilename) && $cacheTime < 86400) {
			// file is valid
			$out.='<div>Using cached version of '.$cacheFilename.'</div>';
			$xmlDoc = simplexml_load_file($cacheFilename);
			$out.='<div>Loaded cached XML document</div>';
		}
		else {
			// expired or doesn't exist
			$out.='<div>Downloading tvdb xml file</div>';
			$dataUrl = 'http://www.thetvdb.com/api/'.$apiKey.'/series/'.$sIdent.'/all/'.$_POST['lang'].'.xml';
			// download to file
			$dataFile = dowloadUrlToFile($dataUrl, $cacheFilename);
			
			$out.='<div>Series XML downloaded to: '.$cacheFilename.'</div>';
			$xmlDoc = simplexml_load_file($cacheFilename);
			$out.='<div>Loaded downloaded XML document</div>';
		}
		
		$episodeData = episodeInfo($xmlDoc, $seaIdent, $epIdent);
		
		//update file type to tv episode
		$out.='<div>Updating file type to TV Episode</div>';
		
		// Update the Episode Type
		mysqli_query($con, "UPDATE File SET FK_MediaSubType=1  WHERE `PK_File`=\"$fileIdent\" ") or die (mysqli_error($con));
		// Update the Resolution
		mysqli_query($con, "UPDATE File SET FK_FileFormat=\"$rez\" WHERE `PK_File`=\"$fileIdent\" ") or die (mysqli_error($con));
		
		// Handle Series info, Add series  Series, Program 
		$out.='<div>Adding Series info: '.$episodeData['series']['series'].'</div>';
		$program = mysqli_real_escape_string($con, $episodeData['series']['series']);
		$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$program\" AND FK_AttributeType=42";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Adding series to Database</div>';
			// insert it 
			$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (42, \"$program\")";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			$series_id = mysqli_insert_id($con);
			$out.='<div>Series id: '.$series_id.'</div>';
		}
		else {
			$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
			$series_id = $row['PK_Attribute'];
			$out.='<div>Found existing series, id: '.$series_id.'</div>';
		}
			
		// Link Series with TV Show
		$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$series_id AND FK_File=$fileIdent";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Linking series with TV show.</div>';
			// insert it 
			$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $series_id, 0, 0)";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		}
		else {
			$out.='<div>Series already linked with TV show.</div>';
		}
			
		// Update channel
		$out.='<div>Adding channel info to TV show.</div>';
		$channel = mysqli_real_escape_string($con, $episodeData['series']['network']);
		$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$channel\" AND FK_AttributeType=10";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Adding channel to Database</div>';
			// insert it 
			$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (10, \"$channel\")";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			$channel_id = mysqli_insert_id($con);
			$out.='<div>Channel id: '.$channel_id.'</div>';
		}
		else {
			$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
			$channel_id = $row['PK_Attribute'];
			$out.='<div>Found existing channel, id: '.$channel_id.'</div>';
		}	
			
		// Link Channel with TV Show
		$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$channel_id AND FK_File=$fileIdent";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Linking channel with TV show.</div>';
			// insert it 
			$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $channel_id, 0, 0)";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			}
		else {
			$out.='<div>Channel already linked with TV show.</div>';
		}
		
		// Update first aired
		$out.='<div>Adding first aired info to TV show.</div>';
		if (isset($episodeData['first_aired']) && $episodeData['first_aired'] != '') {
			$first_aired = mysqli_real_escape_string($con, $episodeData['first_aired']);
			$sql = "SELECT PK_Attribute, Name, Class FROM Attribute WHERE Class=\"$first_aired\" AND FK_AttributeType=41";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				$out.='<div>Adding first_aired to Database</div>';
				// insert it 
				$sql = "INSERT INTO Attribute (FK_AttributeType, Name, Class) VALUES (41, 'First Aired', \"$first_aired\")";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
				$first_aired_id = mysqli_insert_id($con);
				$out.='<div>First Aired id: '.$first_aired_id.'</div>';
			}
			else {
				$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
				$first_aired_id = $row['PK_Attribute'];
				$out.='<div>Found existing firstaired, id: '.$first_aired_id.'</div>';
			}
				
			// Link First Aired with TV Show
			$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$first_aired_id AND FK_File=$fileIdent";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				$out.='<div>Linking first_aired with TV show.</div>';
				// insert it 
				$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $first_aired_id, 0, 0)";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			}
			else {
				$out.='<div>First_aired already linked with TV show.</div>';
			}
		}
		
		// Update Director 
		$out.='<div>Adding director info to TV show.</div>';
		if (isset($episodeData['director']) && $episodeData['director'] != '') {
			$director = mysqli_real_escape_string($con, $episodeData['director']);
			$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$director\" AND FK_AttributeType=5";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				$out.='<div>Adding director to Database</div>';
				// insert it 
				$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (5, \"$director\")";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
				$director_id = mysqli_insert_id($con);
				$out.='<div>Director id: '.$director_id.'</div>';
			}
			else {
				$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
				$director_id = $row['PK_Attribute'];
				$out.='<div>Found existing director, id: '.$director_id.'</div>';
			}	
			
			// Link Director with TV Show
			$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$director_id AND FK_File=$fileIdent";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				$out.='<div>Linking director with TV show.</div>';
				// insert it 
				$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $director_id, 0, 0)";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			}
			else {
				$out.='<div>Director already linked with TV show.</div>';
			}
		}
		
		
		// Update Actors
		$out.='<div>Adding Actor info to TV show.</div>';
		if (isset($episodeData['series']['actors']) && count($episodeData['series']['actors'] > 0)) {
			foreach ($episodeData['series']['actors'] as $actor_info) {
				$out.='<div>Adding actor: '.$actor_info['name'].'</div>';
				$actor = mysqli_real_escape_string($con, $actor_info['name']);
				$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$actor\" AND FK_AttributeType=6";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
				if (mysqli_num_rows($result)==0) {
					$out.='<div>Adding actor to Database</div>';
					// insert it 
					$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (6, \"$actor\")";
					$result = mysqli_query($con, $sql) or die(mysqli_error($con));
					$actor_id = mysqli_insert_id($con);
					$out.='<div>Actor id: '.$actor_id.'</div>';
				}
				else {
					$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
					$actor_id = $row['PK_Attribute'];
					$out.='<div>Found existing actor, id: '.$actor_id.'</div>';
				}
					
				// check if actor has image
				if ($actor_info['image'] != '') {
					// grab actor image, store it
					$actorImage = $actor_info['image'];
					
					$out.='<div>Actor Image: '.$actorImage.'</div>';
					
					// check if we have image for actor already
					$sql = "SELECT PK_Picture FROM Picture_Attribute WHERE FK_Attribute=$actor_id";
					$result = mysqli_query($con, $sql) or die(mysqli_error($con));
					if (mysqli_num_rows($result)==0) {
						// save the image
						$actorImageURL = "http://www.thetvdb.com/banners/".$actorImage;
						$fileName = rand(1000,20000).rand(1000,20000).".jpg";
						
						// Download
						$ch = curl_init();
						curl_setopt($ch, CURLOPT_URL, $actorImageURL);
						curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
						curl_setopt($ch, CURLOPT_BINARYTRANSFER, 1);
						curl_setopt($ch, CURLOPT_TIMEOUT, 200);
						curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 200);
						$rawdata = curl_exec($ch);
						curl_close($ch);
						$file = APPROOT."mediapics/".$fileName;
						$sfile = APPROOT."mediapics/".$fileName."_tn.jpg";
						file_put_contents($file, $rawdata);
						
						// Create thumb
						exec("/usr/local/bin/convert " . $file . " -resize x100 " . $sfile);
						
						// create db entry for image
						$sql = "INSERT INTO Picture (FK_PictureType, Extension, URL) VALUES (1, 'jpg', '')";
						mysqli_query($con, $sql) or die(mysqli_error($con));
						$picture_id = mysqli_insert_id($con);
						
						// rename file to picture id
						rename($file, APPROOT."mediapics/".$picture_id.".jpg");
						rename($sfile, APPROOT."mediapics/".$picture_id."_tn.jpg");
						
						// Link Actor and Picture
						$sql = "INSERT INTO Picture_Attribute (FK_Picture, FK_Attribute) VALUES ($picture_id, $actor_id)";
						mysqli_query($con, $sql) or die(mysqli_error($con));
					
					}
				}
				
				// Link Actor with TV Show
				$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$actor_id AND FK_File=$fileIdent";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
				if (mysqli_num_rows($result)==0) {
					$out.='<div>Linking actor with TV show.</div>';
					// insert it 
					$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $actor_id, 0, 0)";
					$result = mysqli_query($con, $sql) or die(mysqli_error($con));
				}
				else {
					$out.='<div>Actor already linked with TV show.</div>';
				}
			}
		}
			
		// Update Title (episode name)
		$out.='<div>Adding Title info to TV show.</div>';
		$title = mysqli_real_escape_string($con, $episodeData['episodename']);
		$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$title\" AND FK_AttributeType=1";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Adding title to Database</div>';
			// insert it 
			$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (1, \"$title\")";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			$title_id = mysqli_insert_id($con);
			$out.='<div>Title id: '.$title_id.'</div>';
		}
		else {
			$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
			$title_id = $row['PK_Attribute'];
			$out.='<div>Found existing title, id: '.$title_id.'</div>';
		}	
			
		// Link Title with TV Show
		$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$title_id AND FK_File=$fileIdent";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Linking title with TV show.</div>';
			// insert it 
			$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $title_id, 0, 0)";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		}
		else {
			$out.='<div>Title already linked with TV show.</div>';
		}
			
		// Update Season
		$out.='<div>Adding season info to TV show.</div>';
		$season = mysqli_real_escape_string($con, $seaIdent);
		$sql = "SELECT PK_Attribute, Name, Class FROM Attribute WHERE Class=\"$season\" AND FK_AttributeType=48";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Adding season to Database</div>';
			// insert it 
			$sql = "INSERT INTO Attribute (FK_AttributeType, Name, Class) VALUES (48, 'Season', \"$season\")";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			$season_id = mysqli_insert_id($con);
			$out.='<div>Season id: '.$season_id.'</div>';
		}
		else {
			$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
			$season_id = $row['PK_Attribute'];
			$out.='<div>Found existing season, id: '.$season_id.'</div>';
		}	
			
		// Link Season with TV Show
		$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$season_id AND FK_File=$fileIdent";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Linking season with TV show.</div>';
			// insert it 
			$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $season_id, 0, 0)";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		}
		else {
			$out.='<div>Season already linked with TV show.</div>';
		}
			
		// Update Episode
		$out.='<div>Adding episode info to TV show.</div>';
		$episode = mysqli_real_escape_string($con, $epIdent);
		$sql = "SELECT PK_Attribute, Name, Class FROM Attribute WHERE Class=\"$episode\" AND FK_AttributeType=49";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Adding episode to Database</div>';
			// insert it 
			$sql = "INSERT INTO Attribute (FK_AttributeType, Name, Class) VALUES (49, 'Episode', \"$episode\")";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			$episode_id = mysqli_insert_id($con);
			$out.='<div>Episode id: '.$episode_id.'</div>';
		}
		else {
			$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
			$episode_id = $row['PK_Attribute'];
			$out.='<div>Found existing episode, id: '.$episode_id.'</div>';
		}	
			
		// Link Episode with TV Show
		$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$episode_id AND FK_File=$fileIdent";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Linking episode with TV show.</div>';
			// insert it 
			$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $episode_id, 0, 0)";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		}
		else {
			$out.='<div>Episode already linked with TV show.</div>';
		}
		
		// Update overview
		$out.='<div>Adding overview info to TV show.</div>';
		$overview = mysqli_real_escape_string($con, $episodeData['overview']);
		$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$overview\" AND FK_AttributeType=17";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Adding overview to Database</div>';
			// insert it 
			$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (17, \"$overview\")";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			$overview_id = mysqli_insert_id($con);
			$out.='<div>Overview id: '.$overview_id.'</div>';
		}
		else {
			$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
			$overview_id = $row['PK_Attribute'];
			$out.='<div>Found existing overview, id: '.$overview_id.'</div>';
		}	
			
		// Link Overview with TV Show
		$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$overview_id AND FK_File=$fileIdent";
		$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		if (mysqli_num_rows($result)==0) {
			$out.='<div>Linking overview with TV show.</div>';
			// insert it 
			$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $overview_id, 0, 0)";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
		}
		else {
			$out.='<div>Overview already linked with TV show.</div>';
		}
		
		
		// Get Episode thumb, its in filename
		$out.='<div>Adding episode/series image thumbnails to TV show.</div>';
		
		// Use episode if we have it
		if ($episodeData['filename'] != '') {
		
			$out.='<div>Episode image: '.$episodeData['filename'].'</div>';
			
			// check if we have image for this TV show already
			$sql = "SELECT PK_Picture FROM Picture_File WHERE FK_File=$fileIdent";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				// save the image
				$imageURL = "http://www.thetvdb.com/banners/".$episodeData['filename'];
				$fileName = rand(1000,20000).rand(1000,20000).".jpg";
				
				// Download
				$ch = curl_init();
				curl_setopt($ch, CURLOPT_URL, $imageURL);
				curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
				curl_setopt($ch, CURLOPT_BINARYTRANSFER, 1);
				curl_setopt($ch, CURLOPT_TIMEOUT, 200);
				curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 200);
				$rawdata = curl_exec($ch);
				curl_close($ch);
				$file = APPROOT."mediapics/".$fileName;
				$sfile = APPROOT."mediapics/".$fileName."_tn.jpg";
				file_put_contents($file, $rawdata);
				
				// Create thumb
				exec("/usr/local/bin/convert " . $file . " -resize x150 " . $sfile);
				
				// create db entry for image
				$sql = "INSERT INTO Picture (FK_PictureType, Extension, URL) VALUES (1, 'jpg', '')";
				mysqli_query($con, $sql) or die(mysqli_error($con));
				$picture_id = mysqli_insert_id($con);
				
				// rename file to picture id
				rename($file, APPROOT."mediapics/".$picture_id.".jpg");
				rename($sfile, APPROOT."mediapics/".$picture_id."_tn.jpg");
				
				// Link TV show and Picture
				$sql = "INSERT INTO Picture_File (FK_Picture, FK_File) VALUES ($picture_id, $fileIdent)";
				mysqli_query($con, $sql) or die(mysqli_error($con));
			}
			else {
				$out.='<div>TV show already has a picture</div>';
			}
		}
		// Fall back to series banner if no episode pic
		else if ($episodeData['series']['banner'] != '') {
		
			$out.='<div>Series image: '.$episodeData['series']['banner'].'</div>';
			
			// check if we have image for this TV show already
			$sql = "SELECT PK_Picture FROM Picture_File WHERE FK_File=$fileIdent";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				// save the image
				$imageURL = "http://www.thetvdb.com/banners/".$episodeData['series']['banner'];
				$fileName = rand(1000,20000).rand(1000,20000).".jpg";
				
				// Download
				$ch = curl_init();
				curl_setopt($ch, CURLOPT_URL, $imageURL);
				curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
				curl_setopt($ch, CURLOPT_BINARYTRANSFER, 1);
				curl_setopt($ch, CURLOPT_TIMEOUT, 200);
				curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 200);
				$rawdata = curl_exec($ch);
				curl_close($ch);
				$file = APPROOT."mediapics/".$fileName;
				$sfile = APPROOT."mediapics/".$fileName."_tn.jpg";
				file_put_contents($file, $rawdata);
				
				// Create thumb
				exec("/usr/local/bin/convert " . $file . " -resize x150 " . $sfile);
				
				// create db entry for image
				$sql = "INSERT INTO Picture (FK_PictureType, Extension, URL) VALUES (1, 'jpg', '')";
				mysqli_query($con, $sql) or die(mysqli_error($con));
				$picture_id = mysqli_insert_id($con);
				
				// rename file to picture id
				rename($file, APPROOT."mediapics/".$picture_id.".jpg");
				rename($sfile, APPROOT."mediapics/".$picture_id."_tn.jpg");
				
				// Link TV show and Picture
				$sql = "INSERT INTO Picture_File (FK_Picture, FK_File) VALUES ($picture_id, $fileIdent)";
				mysqli_query($con, $sql) or die(mysqli_error($con));
			}
			else {
				$out.='<div>TV show already has a picture</div>';
			}
		}
		else {
			$out.='<div>TV show does not have a picture from TVDB</div>';
		}
		
		// series genre, need to clean this up
		$out.='<div>Adding genre info to TV show.</div>';
		$genre_string = $episodeData['series']['genre'];
		
		// first remove | from end of string
		if (substr($genre_string, -1) == '|') {
			$genre_string = substr($genre_string, 0, -1);
		}
		
		// split on |
		$genres = explode("|", $genre_string);
		
		foreach($genres as $genre_name) {
			
			if ($genre_name == '') {
				continue;
			}
			
			$out.='<div>Adding genre: '.$genre_name.'</div>';
			
			$genre = mysqli_real_escape_string($con, $genre_name);
			$sql = "SELECT PK_Attribute, Name FROM Attribute WHERE Name=\"$genre\" AND FK_AttributeType=8";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				$out.='<div>Adding genre to Database</div>';
				// insert it 
				$sql = "INSERT INTO Attribute (FK_AttributeType, Name) VALUES (8, \"$genre\")";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
				$genre_id = mysqli_insert_id($con);
				$out.='<div>Genre id: '.$genre_id.'</div>';
			}
			else {
				$row = mysqli_fetch_array($result, MYSQLI_ASSOC);
				$genre_id = $row['PK_Attribute'];
				$out.='<div>Found existing genre, id: '.$genre_id.'</div>';
			}
				
			// Link Genre with TV Show
			$sql = "SELECT FK_Attribute, FK_File FROM File_Attribute WHERE FK_Attribute=$genre_id AND FK_File=$fileIdent";
			$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			if (mysqli_num_rows($result)==0) {
				$out.='<div>Linking genre with TV show.</div>';
				// insert it 
				$sql = "INSERT INTO File_Attribute (FK_File, FK_Attribute, Track, Section) VALUES ($fileIdent, $genre_id, 0, 0)";
				$result = mysqli_query($con, $sql) or die(mysqli_error($con));
			}
			else {
				$out.='<div>Genre already linked with TV show.</div>';
			}
			
		}
		
		$out.='<div>Finished Processing TV Show.</div>';
		}
		}
		$out.='</td></table>';
	
}
// NO ACTION YET
else
{
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/editMediaFile.lang.php');

	$out.='<ul>
		<li><form method="post" action="index.php">
			<input type="hidden" name="section" value="tvdbbatch">
			<input type="hidden" name="action" value="scan">
			<input type="submit" value="Scan for TV Shows">
			</form>
		</li></ul>';

}

	$output->setReloadLeftFrame(false);
	$output->setMenuTitle('Check TVDB');
	$output->setPageTitle('TVDB Batch Updater');
	$output->setScriptInHead($scriptInHead);	
	$output->setScriptInBody('bgColor="#F0F3F8"');
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME);			
	$output->output();

}//end primary function

function dowloadUrlToFile($url, $dest) {

	if ($url == '')
		return false;
	
	$ch = curl_init();
	curl_setopt($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_BINARYTRANSFER, 1);
	curl_setopt($ch, CURLOPT_TIMEOUT, 200);
	curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 200);
	$rawdata = curl_exec($ch);
	curl_close($ch);
	file_put_contents($dest, $rawdata);
	
	return filesize($dest);
}

function episodeInfo(&$xmlObj, $season, $episode) {
	
	$result = array();
	
	// Get Series Data
	$result['series']['id'] = $xmlObj->Series->id;
	$result['series']['language'] = $xmlObj->Series->Language;
	$result['series']['series'] = $xmlObj->Series->SeriesName;
	$result['series']['overview'] = $xmlObj->Series->Overview;
	$result['series']['banner'] = $xmlObj->Series->banner;
	$result['series']['network'] = $xmlObj->Series->Network;
	$result['series']['rating'] = $xmlObj->Series->Rating;
	$result['series']['genre'] = $xmlObj->Series->Genre;
	
	// Get Actors
	$result['series']['actors'] = array();
	foreach ($xmlObj->Series->Actors->Actor as $actor) {
		$actor_array = array('name' => $actor->Name, 'role' => $actor->Role, 'image' => $actor->Image);
		$result['series']['actors'][] = $actor_array;
	}
	
	// Get Episode Data
	foreach ($xmlObj->Episode as $ep) {
		if ($ep->SeasonNumber == $season && $ep->EpisodeNumber == $episode) {
			
			$result['id'] = $ep->id;
			$result['season'] = $ep->SeasonNumber;
			$result['episode'] = $ep->EpisodeNumber;
			$result['director'] = $ep->Director;
			$result['episodename'] = $ep->EpisodeName;
			$result['first_aired'] = $ep->FirstAired;
			$result['overview'] = $ep->Overview;
			$result['filename'] = $ep->filename;
			$result['season_id'] = $ep->seasonid;
			$result['series_id'] = $ep->seriesid;
			
			break;
		}
	}
	
	return $result;
}
?>