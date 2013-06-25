<?php

namespace Application\Classes;

class OutputParser
{
    protected $closeSpan;
    protected $isInsideSpan;
    protected $isBold;
    protected $isUnderlined;
    protected $fgColor;
    protected $fgColorType;
    protected $bgColor;
    protected $bgColorType;

    const COLOR_16 = 1;

    protected $color16 = array(
        /// dark
        '0' => array(
            '0' => '000000', /// black
            '1' => 'cd0000', /// red
            '2' => '00cd00', /// green
            '3' => 'cdcd00', /// brown
            '4' => '0000ee', /// blue
            '5' => 'cd00cd', /// magenta
            '6' => '00cdcd', /// cyan
            '7' => 'e5e5e5', /// gray
        ),

        /// bright
        '1' => array(
            '0' => '7f7f7f', /// dark gray
            '1' => 'ff0000', /// red
            '2' => '00ff00', /// green
            '3' => 'ffff00', /// yellow
            '4' => '5c5cff', /// blue
            '5' => 'ff00ff', /// magenta
            '6' => '00ffff', /// cyan
            '7' => 'ffffff', /// white
        ),
    );

    protected function clear()
    {
        $this->closeSpan = true;
        $this->isBold = false;
        $this->isUnderlined = false;
        $this->fgColor = null;
        $this->fgColorType = static::COLOR_16;
        $this->bgColor = null;
        $this->bgColorType = static::COLOR_16;
    }

    public function parse($text)
    {
        $this->isInsideSpan = false;
        $this->clear();
        $esc = pack('C', 27);

        $text = preg_replace_callback(
            "#{$esc}\[(.*?)m#",
            function ($matches) {
                $text = '';
                $original = $matches[0];

                $matches = explode(';', $matches[1]);
                foreach ($matches as $code) {
                    $code = (int) $code;
                    if ($code == 0) {
                        $this->clear();
                    } elseif ($code == 1) {
                        $this->isBold = true;
                    } elseif ($code == 2) {
                        $this->isUnderlined = true;
                    } elseif ($code >= 30 && $code <= 37) {
                        $this->fgColor = ($code - 30) . '';
                        $this->fgColorType = static::COLOR_16;
                    } elseif ($code >= 40 && $code <= 47) {
                        $this->bgColor = ($code - 40) . '';
                        $this->bgColorType = static::COLOR_16;
                    }
                }

                $text .= $this->closeSpan();

                $css = $this->getCSS();
                if ($css !== '') {
                    $text .= '<span style="' . $css . '">';
                    $this->isInsideSpan = true;
                }

                if ($text === '') {
                    $text = $original;
                }

                return $text;
            },
            $text
        );

        $text .= $this->closeSpan();
        return $text;
    }

    protected function getCSS()
    {
        $css = '';

        if ($this->isBold == true) {
            $css .= 'font-weight:bold;';
        }

        if ($this->isUnderlined == true) {
            $css .= 'font-decoration:underline;';
        }

        if ($this->fgColor !== null) {
            switch ($this->fgColorType) {
                case static::COLOR_16:
                    $brightness = ($this->isBold == true) ? '1' : '0';
                    $css .= 'color:#' . $this->color16[$brightness][$this->fgColor] . ';';
                    break;
            }
        }

        if ($this->bgColor !== null) {
            switch ($this->bgColorType) {
                case static::COLOR_16:
                    $brightness = '0';
                    $css .= 'background-color:#' . $this->color16[$brightness][$this->bgColor] . ';';
                    break;
            }
        }

        return $css;
    }

    protected function closeSpan()
    {
        if ($this->isInsideSpan == true) {
            $this->isInsideSpan = false;
            return '</span>';
        }

        return '';
    }
}
