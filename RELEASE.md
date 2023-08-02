# Releasing the MongoDB BI Connector ODBC Driver

This document describes the version policy and release process for the MongoDB BI Connector ODBC Driver.

## Release process

### Pre-Release Tasks

#### Start Release Ticket
Move the JIRA ticket for the release to the "In Progress" state.
Ensure that its fixVersion matches the version being released.

#### Complete the Release in JIRA
Go to the [BIC releases page](https://jira.mongodb.org/projects/BI?selectedItem=com.atlassian.jira.jira-projects-plugin%3Arelease-page&status=unreleased), and ensure that all the tickets in the fixVersion to be released are closed.
Ensure that all the tickets have the correct type. Take this opportunity to edit ticket titles if they can be made more descriptive.
The ticket titles will be published in the changelog.

If you are releasing a patch version but a ticket needs a minor bump, update the fixVersion to be a minor version bump.
If you are releasing a patch or minor version but a ticket needs a major bump, stop the release process immediately.

The only uncompleted ticket in the release should be the release ticket.
If there are any remaining tickets that will not be included in this release, remove the fixVersion and assign them a new one if appropriate.

Close the release on JIRA, adding the current date (you may need to ask the SQL project manager to do this).

#### Ensure Evergreen Passing
Ensure that the build you are releasing is passing the tests on the evergreen waterfall.

### Release Tasks

#### Ensure master up to date
Ensure you have the `master` branch checked out, and that you have pulled the latest commit from `mongodb/mongo-bi-connector-odbc-driver`.

#### Version information update to include in the bump commit 

##### Changelog

The [ChangeLog file](https://github.com/mongodb/mongo-odbc-driver/blob/master/ChangeLog) should be updated with release notes.

##### version.cmake
The following variables should be changed as appropriate in [version.cmake](https://github.com/mongodb/mongo-odbc-driver/blob/master/version.cmake)  :
- CONNECTOR_MAJOR
- CONNECTOR_MINOR
- CONNECTOR_PATCH
- CONNECTOR_QUALITY (RC, GA, etc)

##### VERSION.txt
Update [VERSION.txt](https://github.com/mongodb/mongo-odbc-driver/blob/master/mongodb-odbc-driver/bin/VERSION.txt) with the current version.  
Note: Do not add newlines, as this is catted directly into files.

##### BinaryFragment.wxs
Update [BinaryFragment.wxs](https://github.com/mongodb/mongo-odbc-driver/blob/master/mongodb-odbc-driver/installer/msi/BinaryFragment.wxs) and change the ODBC driver name for both the ansi and unicode driver.
```
<ODBCDriver File="mdbodbca.dll" Id="MongoDB_ODBC_X.Ya_Driver" Name="MongoDB ODBC X.Y.Z ANSI Driver" SetupFile="mdbodbcS.dll" />
```
```
ODBCDriver File="mdbodbcw.dll" Id="MongoDB_ODBC_X.Yw_Driver" Name="MongoDB ODBC X.Y.Z Unicode Driver" SetupFile="mdbodbcS.dll" />
```

##### setupgui/windows/odbcdialogparams.rc
Update the following variables in [setupgui/windows/odbcdialogparams.rc](https://github.com/mongodb/mongo-odbc-driver/blob/master/setupgui/windows/odbcdialogparams.rc):
- FileDescription
- ProductName
- SpecialBuild (if necessary)

##### Windows installers
Major and minor releases require the most change as X.Y can be seen in many of the documentation files with the release.  
- Update X.Y accordingly in [mongodb-odbc-driver/installer/msi/build-msi.ps1](https://github.com/mongodb/mongo-odbc-driver/blob/master/mongodb-odbc-driver/installer/msi/build-msi.ps1)
- The upgrade code in “mongodb-odbc-driver/installer/msi/build-msi.ps1” must also be updated. A good place to generate new UUIDs is https://www.uuidgenerator.net/.  Another simple option is to run this on a command line (assuming python is installed):
$ python -c "import uuid; print str(uuid.uuid4())"

#### Create the tag and push
Once the updates above have been made, commit and tag it for release.
```
git commit -am "BUMP vX.Y.Z"
git tag -a -m "X.Y.Z" vX.Y.Z <githash>
```
Once tagged, push the commit - and newly created tag - to master
```
git push && git push --tags
```

#### Attach the relevant binaries from Evergreen to the Github release page.
Attach the following to the release under Assets :
- mongodb-connector-odbc-X.Y.Z-macos-64-bit.dmg
- mongodb-connector-odbc-X.Y.Z-rhel-7.0-64.tar.gz
- mongodb-connector-odbc-X.Y.Z-ubuntu-14.04-64.tar.gz
- mongodb-connector-odbc-X.Y.Z-ubuntu-16.04-64.tar.gz
- mongodb-connector-odbc-X.Y.Z-win-32-bit.msi
- mongodb-connector-odbc-X.Y.Z-win-64-bit.msi
They should all point to the download center https://github.com/mongodb/mongo-bi-connector-odbc-driver/releases/download/vX.Y.Z/mongodb-connector-odbc-X.Y.Z-{platform}.{ext}


### Note
The following files:
- mongodb-odbc-driver/installer/msi/README.rtf link
- mongodb-odbc-driver/installer/msi/THIRD-PARTY-NOTICES.rtf link
- mongodb-odbc-driver/installer/dmg/resources/README.rtf link
- mongodb-odbc-driver/installer/dmg/resources/THIRD-PARTY-NOTICES.rtf link

should be kept in sync with README and Licenses_for_Third-Party_Components.txt in the root directory, respectively, for the msi and dmg installers.  
These are installed by the msi and dmg installer and thus should be rtf files so that they are easily readable on Windows (they will open with Wordpad instead of Notepad). 
This is also a common format on macOS, so it is appropriate.
