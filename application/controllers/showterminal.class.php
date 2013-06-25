<?php

namespace Application\Controllers;

use \WebFW\Core\HTMLController;
use \Application\Classes\TTYClient;

class ShowTerminal extends HTMLController
{
    public function __construct()
    {
        parent::__construct();
        $this->pageTitle = 'Terminal';
    }
    public function execute()
    {
        if (TTYClient::getInstance()->isConnectionEstablished() === false) {
            var_dump(TTYClient::getInstance()->getLastError());
        } else {
            //var_dump(TTYClient::getInstance()->read());
            var_dump("done");
        }
        TTYClient::getInstance()->disconnect();
        \WebFW\Core\SessionHandler::kill("tty");
    }
}
