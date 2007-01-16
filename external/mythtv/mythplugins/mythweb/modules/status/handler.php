<?php
/**
 * A straight passthrough for the mythbackend status web page (usually on
 * port 6544)
 *
 * @url         $URL: http://svn.mythtv.org/svn/branches/release-0-20-fixes/mythplugins/mythweb/modules/status/handler.php $
 * @date        $Date: 2006-06-24 22:03:10 +0300 (Sat, 24 Jun 2006) $
 * @version     $Revision: 10290 $
 * @author      $Author: xris $
 * @license     GPL
 *
 * @package     MythWeb
 * @subpackage  Status
 *
/**/

// Get the address/port of the master machine
    $masterhost = get_backend_setting('MasterServerIP');
    $statusport = get_backend_setting('BackendStatusPort');

// XML mode?
    $xml_param = ($Path[1] == 'xml') ? '/xml' : '';

// Make sure the content is interpreted as UTF-8
    header('Content-Type:  text/html; charset=UTF-8');

// Load the status page
    if (function_exists('file_get_contents'))
        $status = file_get_contents("http://$masterhost:$statusport$xml_param");
    else
        $status = implode("\n", file("http://$masterhost:$statusport$xml_param"));

// Extract the page title
    preg_match('#<title>(.+?)</title>#s', $status, $title);
    $title = $title[1];

// Clean up the page, and add some invisible content with the actual URL grabbed
    $status = "<!-- Obtained from:  http://$masterhost:$statusport -->\n"
             .preg_replace('#\s*</body>\s*</html>\s*$#s', '',
                  preg_replace('/^.+?<body>\s*\n/s', "\n",
                      $status
                  )
              );

// Print the status page template
    require_once tmpl_dir.'status.php';

// Yup, that really is it.
    exit;
