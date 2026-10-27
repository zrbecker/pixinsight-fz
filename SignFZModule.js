/*
 * SignFZModule.js - create a local signing keys file (first run only) and sign FZ-pxm.dll.
 *
 * Run in PixInsight: SCRIPT > Execute Script File..., pick this file. It signs the
 * FZ-pxm.dll in the same folder as this script.
 * You choose the password; it never leaves this dialog. The same password unlocks the
 * keys file later (Local Signing Identity dialog, and re-signing after an update).
 *
 * Uses the same Security calls as PixInsight's own SigningKeys and CodeSign scripts:
 * generateLocalSigningKeysFile, loadSigningKeysFile, generateModuleSignatureFile.
 * A local signing identity is tied to your PixInsight licence, so the signature only
 * works on machines where your licence is activated.
 */

#include <pjsr/Sizer.jsh>
#include <pjsr/StdButton.jsh>
#include <pjsr/StdIcon.jsh>

#define TITLE "Sign FZ module"

var MODULE_DIR  = File.extractDrive( #__FILE__ ) + File.extractDirectory( #__FILE__ );
var MODULE_FILE = MODULE_DIR + "/FZ-pxm.dll";
var KEYS_DIR    = File.homeDirectory + "/PixInsight-signing";   // outside the module folder: it holds your private key
var KEYS_FILE   = KEYS_DIR + "/local-signing.xssk";

function PasswordDialog( creating )
{
   this.__base__ = Dialog;
   this.__base__();
   this.windowTitle = TITLE;

   this.info = new Label( this );
   this.info.useRichText = true;
   this.info.wordWrapping = true;
   this.info.minWidth = 420;
   this.info.text = creating
      ? "<p>No signing keys file yet. One will be created at:<br/><b>" + KEYS_FILE + "</b></p>" +
        "<p>Choose a password: at least 8 characters, an uppercase and a lowercase letter, " +
        "and a digit or punctuation mark. ASCII only, no spaces. <b>Write it down</b>: you need it " +
        "for the Local Signing Identity dialog and every time the module is rebuilt.</p>"
      : "<p>Using the signing keys file:<br/><b>" + KEYS_FILE + "</b></p><p>Enter its password.</p>";

   this.password = new Edit( this );
   this.password.passwordMode = true;
   this.confirm = new Edit( this );
   this.confirm.passwordMode = true;
   this.confirm.visible = creating;

   // Not named ok/cancel: those are Dialog's own methods, and a property of that name hides them.
   this.sign_Button = new PushButton( this );
   this.sign_Button.text = "Sign";
   this.sign_Button.defaultButton = true;
   this.sign_Button.onClick = function() { this.dialog.ok(); };
   this.cancel_Button = new PushButton( this );
   this.cancel_Button.text = "Cancel";
   this.cancel_Button.onClick = function() { this.dialog.cancel(); };

   this.buttons = new HorizontalSizer;
   this.buttons.addStretch();
   this.buttons.add( this.sign_Button );
   this.buttons.addSpacing( 8 );
   this.buttons.add( this.cancel_Button );

   this.sizer = new VerticalSizer;
   this.sizer.margin = 8;
   this.sizer.spacing = 6;
   this.sizer.add( this.info );
   this.sizer.add( this.password );
   if ( creating )
      this.sizer.add( this.confirm );
   this.sizer.add( this.buttons );
   this.adjustToContents();
}
PasswordDialog.prototype = new Dialog;

function fail( message )
{
   (new MessageBox( message, TITLE, StdIcon_Error, StdButton_Ok )).execute();
}

function main()
{
   console.show();
   if ( !File.exists( MODULE_FILE ) )
      return fail( "<p>FZ-pxm.dll was not found next to this script:</p><p>" + MODULE_FILE +
                   "</p><p>Keep the script and the DLL in the same folder.</p>" );

   let creating = !File.exists( KEYS_FILE );
   let dialog = new PasswordDialog( creating );
   for ( ;; )
   {
      if ( !dialog.execute() )
         return;
      if ( creating && dialog.password.text != dialog.confirm.text )
      {
         fail( "The two passwords do not match." );
         continue;
      }
      // ByteArrays: the Security calls wipe them after use.
      let password = dialog.password.utf8;
      if ( creating )
      {
         if ( !Security.validPassword( password ) )
         {
            fail( "<p>That password is not accepted.</p><p>At least 8 characters, an uppercase and a " +
                  "lowercase letter, a digit or punctuation mark. ASCII only, no spaces.</p>" );
            continue;
         }
         if ( !File.directoryExists( KEYS_DIR ) )
            File.createDirectory( KEYS_DIR, true );
         Security.generateLocalSigningKeysFile( KEYS_FILE, password );
         console.noteln( "* Signing keys file created: <raw>" + KEYS_FILE + "</raw>" );
         password = dialog.password.utf8;      // the first copy was wiped
      }

      let keys = Security.loadSigningKeysFile( KEYS_FILE, password );
      if ( !keys.valid )
      {
         keys.publicKey.secureFill();
         keys.privateKey.secureFill();
         fail( "Wrong password, or the keys file is not valid." );
         continue;
      }
      let signature = File.changeExtension( MODULE_FILE, ".xsgn" );
      Security.generateModuleSignatureFile( signature, MODULE_FILE, [], keys.developerId, keys.publicKey, keys.privateKey );
      keys.publicKey.secureFill();
      keys.privateKey.secureFill();
      dialog.password.text = dialog.confirm.text = "";
      console.noteln( "* Module signature generated: <raw>" + signature + "</raw>" );
      break;
   }

   console.noteln( "Next: EDIT > Local Signing Identity... with <raw>" + KEYS_FILE + "</raw> (first time only), " +
                   "then PROCESS > Modules > Install Modules... and search <raw>" + MODULE_DIR + "</raw>" );
   (new MessageBox(
      "<p><b>Signed.</b></p>" +
      (creating
         ? "<p><b>1.</b> EDIT &gt; Local Signing Identity... : select<br/>" + KEYS_FILE +
           "<br/>enter the same password, tick <i>Make the local signing identity persistent</i>, OK.</p>"
         : "") +
      "<p><b>" + (creating ? "2" : "1") + ".</b> PROCESS &gt; Modules &gt; Install Modules... : Search the folder<br/>" +
      MODULE_DIR + "<br/>select FZ-pxm.dll, Install. If this folder's module is already installed, " +
      "just restart PixInsight.</p>",
      TITLE, StdIcon_Information, StdButton_Ok )).execute();
}

main();
