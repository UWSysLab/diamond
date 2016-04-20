// Example express application adding the parse-server module to expose Parse
// compatible API routes.

var express = require('express');
var ParseServer = require('parse-server').ParseServer;

var databaseUri = process.env.DATABASE_URI || 'mongodb://ari:cowsarecool@ds053305.mongolab.com:53305/apptestdatabase';

if (!databaseUri) {
  console.log('DATABASE_URI not specified, falling back to localhost.');
}

var api = new ParseServer({
  databaseURI: databaseUri || 'mongodb://ari:cowsarecool@ds053305.mongolab.com:53305/apptestdatabase',
  cloud: process.env.CLOUD_CODE_MAIN || __dirname + '/cloud/main.js',
  appId: process.env.APP_ID || 'WsCxdr90gpqXO5rPQ6FvVJE46X8Tghr48sEUMaDN',
  masterKey: process.env.MASTER_KEY || 'b0ijciCV82QeRguyW5Oc7LKRLwFR5fa0XlFrC7U3', //Add your master key here. Keep it secret!
  fileKey: 'b78bc6f5-385b-4cf8-8bee-a17c53765d08',
  clientKey: 'qessDyZkCChhXdKo6BZRhiFk8ZHaKPjJEQDMhVin',
  javaScriptKey: '9RPh1e1wqebYomHIdqxXJWvpX0T907eXTirlP872',
  dotNETKey: 'Ymi6qUFE0vt2EPzxXqeMpgMnDLiA8y20gysRo1VY'
});
// Client-keys like the javascript key or the .NET key are not necessary with parse-server
// If you wish you require them, you can set them as options in the initialization above:
// javascriptKey, restAPIKey, dotNetKey, clientKey

var app = express();

// Serve the Parse API on the /parse URL prefix
var mountPath = process.env.PARSE_MOUNT || '/parse';
app.use(mountPath, api);

// Parse Server plays nicely with the rest of your web routes
app.get('/', function(req, res) {
  res.status(200).send('I dream of being a web site.');
});

var port = process.env.PORT || 1337;
app.listen(port, function() {
    console.log('parse-server-example running on port ' + port + '.');
});

//process.env.MONGOLAB_URI
