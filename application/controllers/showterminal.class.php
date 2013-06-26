<?php

namespace Application\Controllers;

use \WebFW\Core\HTMLController;
use \Application\Classes\TTYClient;
use \Application\Classes\OutputParser;

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
            $text = TTYClient::getInstance()->read();
            TTYClient::getInstance()->write("ls -l\n");
            $text .= TTYClient::getInstance()->read();

            $parser = new OutputParser();
            $text = $parser->parse($text);
            echo '<pre style="color: white; background-color: black; font-family: \'DejaVu Sans Mono\', monospace;">' . $text . '</pre>';
        }
        TTYClient::getInstance()->disconnect();
        \WebFW\Core\SessionHandler::kill("tty");
    }

}
