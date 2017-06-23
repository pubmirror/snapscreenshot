<?php
//TITLE=Linux console screenshooter

$title = 'Screenshot program for Linux text console(s)';
$progname = 'snapscreenshot';

function usagetext($prog)
{
  exec($prog.' --help', $kk);
  $k='';foreach($kk as $s)$k.="$s\n";
  return $k;
}

$text = array(
   '1. Purpose' => "

<pre class=smallerpre>".htmlspecialchars(usagetext('/usr/local/bin/snapscreenshot'))."</pre>
<!--br>
Example screenshots:
<a href=\"http://oktober.stc.cx/kala/radio2.jpg\">radio2.jpg</a> (100 kB),
<a href=\"http://oktober.stc.cx/kala/full.png\">full.png</a> (250 kB *big*)
-->

", '1. Copying' => "

snapscreenshot has been written by Joel Yliluoma, a.k.a.
<a href=\"http://iki.fi/bisqwit/\">Bisqwit</a>,<br>
and is distributed under the terms of the
<a href=\"http://www.gnu.org/licenses/licenses.html#GPL\">General Public License</a> (GPL).
<p>
If you happen to see this program useful for you, I'd
appreciate if you tell me :) Perhaps it would motivate
me to enhance the program.

", '1. FAQ' => "

<b>Q</b>:
 I have 132x50 text mode with 8x8 font, and when I take
 a snapshot, it looks very wide and not tall... Why does
 snapscreenshot mess the ratio?<br>
<b>A</b>:
 snapscreenshot doesn't save or even know about dot-per-inch
 ratios, and neither do most image display programs.<br>
 132x50 with 8x8 font means 1054x400 pixel resolution,
 and you clearly can see that 1054 is over two times 400.<br>
 So it really is wider than its tall.
 Your monitor just squashes the pixels horizontally.<br>
 If you want a regular 1.25 ratio image, you have to either
 squeeze it horizontally or stretch it vertically by
 using an image conversion program like Imagemagick.<br>

", '1. Requirements' => "

GNU make is probably required.<br>
The program only works in Linux (I don't know
any other unix clones having /dev/vcs* -devices).

");
include '/WWW/progdesc.php';
