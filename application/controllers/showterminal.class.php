<?php

namespace Application\Controllers;

use \WebFW\Core\HTMLController;

class ShowTerminal extends HTMLController
{
    public function __construct()
    {
        parent::__construct();
        $this->pageTitle = 'Terminal';
    }
    public function execute()
    {
        \Application\Classes\TTYClient::getInstance();
        \WebFW\Core\SessionHandler::kill("tty");
    }
}
