<?php
session_start();

if (!file_exists('../webfw/config/bootstrap.inc.php'))
{
   echo 'WebFW: FATAL ERROR: Bootstrap function unreachable: ' . realpath('../webfw/config/bootstrap.inc.php');
   die;
}

require_once('../webfw/config/bootstrap.inc.php');

\WebFW\Config\Bootstrap();

try
{
   \WebFW\Core\Framework::Start();
}
catch (\WebFW\Core\Exception $e)
{
   $e->ErrorMessage();
   die;
}

?>
