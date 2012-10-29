<?php
/*
 * index.php
 * 
 * Copyright 2012 Benjamin Höglinger <benjamin@nefarius.at>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
ini_set("display_errors", "yes");

session_start();

if (!isset($_SESSION["origURL"]) && isset($_GET["referer"]))
	$_SESSION["origURL"] = $_GET["referer"];

if(isset($_POST['username']) && isset($_POST['password']))
{
	// Create (connect to) SQLite database in file
	$file_db = new PDO('sqlite:/var/lib/scd/scd.sqlite3');
	//$file_db = new PDO('sqlite:/var/www/scd/scd.db');

	// Set errormode to exceptions
	$file_db->setAttribute(PDO::ATTR_ERRMODE, 
							 PDO::ERRMODE_EXCEPTION);
							 
	$db_create = "CREATE TABLE IF NOT EXISTS scd_sessions " .
			"(sid INTEGER PRIMARY KEY, " .
			"ip TEXT NOT NULL, user TEXT NOT NULL, " .
			"last_visit INTEGER NOT NULL)";
					
	$file_db->exec($db_create);
	
	$db_insert = "INSERT INTO scd_sessions " .
			"(ip, user, last_visit) " .
			"VALUES (:ip, :user, :last_visit)";
				
	$stmt = $file_db->prepare($db_insert);
	
	// Bind parameters to statement variables
    $stmt->bindParam(':ip', $ip_address);
    $stmt->bindParam(':user', $username);
    $stmt->bindParam(':last_visit', $timestamp);
    
    $ip_address = $_SERVER['REMOTE_ADDR'];
    $username = $_POST['username'];
    $timestamp = time();
    
    $stmt->execute();
	
	header("Location: ".$_SESSION["origURL"]);
	die();
}

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">

<head>
	<title>SCD Authentication page</title>
	<meta http-equiv="content-type" content="text/html;charset=utf-8" />
	<meta name="generator" content="Geany 0.21" />
	<link rel="stylesheet" type="text/css" media="screen" href="images/Azulmedia.css" />
</head>

<body>
<!-- wrap starts here -->
<div id="wrap">

	<div id="header">	
		
		<h1 id="logo">The <span class="gray">SCD</span> Gateway</h1>	
		<h2 id="slogan">To Secure, Contain, and Deny</h2>		
		
		<!-- Menu Tabs -->
		<div id="menu">
			<ul>
			<li id="current"><a href="index.html">Authentication</a></li>
			<li><a href="index.html">Support</a></li>
			<li><a href="index.html">About</a></li>			
			</ul>		
		</div>		
	
	</div>
				
	<!-- content-wrap starts here -->
	<div id="content-wrap">	 
	
		<div id="main">
		
			<a name="TemplateInfo"></a>			
			<div class="box">
				
				<h1>Authentication <span class="gray">required</span></h1>
				
                <p>You have tried to establish an internet connection. Access to internet resources is prohibited to unauthorised personnel. To continue please verify your identity by entering a valid username and password combination in the login box below:</p>

                <form id="auth_form" method="post" action="">
                  <p>
                    <label for="username">Username</label>
                    <input type="text" name="username" id="username" />
                  </p>
                  <p>
                    <label for="password">Password</label>
                    <input type="password" name="password" id="password" />
                  </p>
                  <p>
                    <input type="submit" name="login" id="login" value="Login" />
                  </p>
                </form>	
			</div>
		</div>
	  <br />
	  
	  <div id="sidebar" >

			<h2 class="clear">I'm having difficulties</h2>
			<ul class="sidemenu">
				<li><a href="#">I forgot my username</a></li>
				<li><a href="#">I forgot my password</a></li>
			</ul>
		</div>
    <div class="clear"></div>
	<!-- content-wrap ends here -->		
	</div>	

<!-- wrap ends here -->
</div>

<!-- footer starts here -->	
<div id="footer-wrap">
	
	<div class="footer-left">
		<p class="align-left">			
		&copy; 2012 Benjamin Höglinger|
		<a href="http://www.bluewebtemplates.com/" title="Website Templates">website templates</a> by <a href="http://www.styleshout.com/">styleshout</a>
       	</p>
	</div>
	
	<div class="footer-right">
		<p class="align-right">
		<a href="index.html">Home</a> |
        <a href="http://validator.w3.org/check/referer">XHTML</a> |
		<a href="http://jigsaw.w3.org/css-validator/check/referer">CSS</a>
		</p>
	</div>
	
</div>
<!-- footer ends here -->
</body>

</html>
