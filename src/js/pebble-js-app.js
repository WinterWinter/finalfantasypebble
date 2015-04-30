Pebble.addEventListener("ready",
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
     Pebble.openURL("http://winterwinter.github.io/finalfantasypebble/"); 
  }
                         );

Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var Heroes = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(Heroes));
    
    var dictionary = {
      "KEY_HERO1" : Heroes.hero1,
      "KEY_HERO2" : Heroes.hero2,
      "KEY_HERO3" : Heroes.hero3,
      "KEY_HERO4" : Heroes.hero4,
      "KEY_BLUETOOTH" : Heroes.bluetooth
       };

    //Send to Pebble, persist there
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log("Sending settings data...");
      },
      function(e) {
        console.log("Settings feedback failed!");
      }
    );
  }
);
