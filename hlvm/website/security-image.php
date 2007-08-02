<?php
  class SecurityImage 
  {
    var $oImage;
    var $iWidth;
    var $iHeight;
    var $iNumChars;
    var $iNumLines;
    var $iSpacing;
    var $sCode;

    function SecurityImage(
      $iWidth = 200,
      $iHeight = 40,
      $iNumChars = 7,
      $iNumLines = 30
    ) 
    { 
      // get parameters
      $this->iWidth = $iWidth;
      $this->iHeight = $iHeight;
      $this->iNumChars = $iNumChars;
      $this->iNumLines = $iNumLines;
             
      // create new image
      $this->oImage = imagecreate($iWidth, $iHeight);
             
      // allocate white background colour
      imagecolorallocate($this->oImage, 255, 255, 255);
             
      // calculate spacing between characters based on width of image
      $this->iSpacing = (int)($this->iWidth / $this->iNumChars);       
    }

    function DrawLines() 
    {
      for ($i = 0; $i < $this->iNumLines; $i++) {
         $iRandColour = rand(190, 250);
         $iLineColour = imagecolorallocate($this->oImage, $iRandColour,
                        $iRandColour, $iRandColour);
         imageline($this->oImage, 
                   rand(0, $this->iWidth), 
                   rand(0, $this->iHeight),
                   rand(0, $this->iWidth), 
                   rand(0, $this->iHeight), $iLineColour);
      }
    }

    function GenerateCode() 
    {
      // reset code
      $this->sCode = '';

      // loop through and generate the code letter by letter
      for ($i = 0; $i < $this->iNumChars; $i++) {
         switch (rand(1,2)) {
         case 1: // Upper Case
           $this->sCode .= chr(rand(65,90));
           break;
         case 2: // Lower Case
           $this->sCode .= chr(rand(97,122));
           break;
         }
      }
    }
    function DrawCharacters() 
    {
      // loop through and write out selected number of characters
      for ($i = 0; $i < strlen($this->sCode); $i++) {
         // select font
         $iCurrentFont = 9;
               
         // select random greyscale colour
         $iRandColour = rand(0, 32);
         $iTextColour = imagecolorallocate($this->oImage, $iRandColour,
           $iRandColour, $iRandColour);
               
         // write text to image
         imagestring($this->oImage, $iCurrentFont, 
           $this->iSpacing / 3 + $i * $this->iSpacing, 
           ($this->iHeight - imagefontheight($iCurrentFont)) / 2,
           $this->sCode[$i], $iTextColour);
      }
    }

    function Create($sFilename = '') 
    {
      // check for existence of GD GIF library
      if (!function_exists('imagegif')) {
         return false;
      }
             
      $this->DrawLines();
      $this->GenerateCode();
      $this->DrawCharacters();
             
      // write out image to file or browser
      if ($sFilename != '') {
         // write stream to file
         imagegif($this->oImage, $sFilename);
      } else {
         // tell browser that data is gif
         header('Content-type: image/gif');
               
         // write stream to browser
         imagegif($this->oImage);
      }
             
      // free memory used in creating image
      imagedestroy($this->oImage);
             
      return true;
    }

    function GetCode()
    {
      return $this->sCode;
    }
  }

  // start PHP session
  session_start();
   
  // get parameters
  isset($_GET['width']) ? $iWidth = (int)$_GET['width'] : $iWidth = 150;
  isset($_GET['height']) ? $iHeight = (int)$_GET['height'] : $iHeight = 30;
   
  // create new image
  $oSecurityImage = new SecurityImage($iWidth, $iHeight);
  if ($oSecurityImage->Create()) {
     // assign corresponding code to session variable  
     // for checking against user entered value
     $_SESSION['secode'] = $oSecurityImage->GetCode();
  } else {
     echo 'Image GIF library is not installed.';
  }
?>
