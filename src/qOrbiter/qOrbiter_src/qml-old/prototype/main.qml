import QtQuick 1.1
import "components"
import "js/ComponentLoader.js" as MyJs

Item {
    id: item
    width:manager.appWidth
    height:manager.appHeight
    focus:true
    property int screensaverTimer:manager.screenSaverTimeout*1000
    Rectangle{
        id:canary
        height: 1
        width: 1
        color:"white"
    }

    Timer{
        id:refresh
        interval: 500
        running:true
        repeat: true
        onTriggered: {
            if(canary.rotation===360)
                canary.rotation =1
            else
                canary.rotation=(canary.rotation+1)
        }
    }

    Component.onCompleted: forceActiveFocus()
    Connections{
        target:manager
        onBackButtonPressed:console.log("Go Back Button!")
    }
    Keys.onReleased:{

        switch(event.key){
        case Qt.Key_Menu:
            console.log("menu button caught in root!")
            break;
            //        case Qt.Key_VolumeUp:
            //            console.log("Vol up")
            //            break;
            //        case Qt.Key_VolumeDown:
            //            console.log("vol down")
            //            break;
        case Qt.Key_MediaPrevious:
            console.log("Caught back button! Phew!")
            manager.goBacktoQScreen()
            break;
        case Qt.Key_Back:
            console.log("Caught Back again! Tricky...")
            manager.goBacktoQScreen()
            break;
        default:
            console.log("I have no idea what key " + event.key + " is. ")
            break;
        }

        event.accepted= true
    }

    function executeBack(){
        if(componentLoader.source !=="")
            componentLoader.source = ""
        else
            componentLoader.source="components/ExitConfirm.qml"
    }

    //    Rectangle{
    //        anchors.fill: parent
    //        gradient: Gradient {
    //            GradientStop { position: 0.0; color: "darkslategrey" }
    //            GradientStop { position: .85; color: "slategrey" }
    //        }
    //    }

    //    ScreenSaver{
    //        id:ss
    //        anchors.fill: parent
    //        MouseArea{
    //            anchors.fill: parent
    //            onClicked: pageLoader.opacity === 0 ?  pageLoader.opacity = 1 : uiTimer.restart()
    //        }
    //    }


    FontLoader{
        id:myFont
        source:"../../../skins-common/fonts/RobotoCondensed-Light.ttf"
    }

    Rectangle{
        anchors.fill: parent
        color:"black"
    }

    Image{
        source:"img/bg.jpg"
        anchors.fill: parent
    }

    signal close()
    signal changeScreen(string s)
    signal setupStart(int x, string y)


    property string locationinfo: "standby"
    property string screenfile
    property string dynamic_height
    property string dynamic_width

    function checkLayout()
    {
        console.log("c++ slot orientation changed")
    }

    Connections{
        target: manager
        onOrientationChanged: checkLayout()
    }

    function scaleX(x){
        return x/100*manager.appWidth
    }
    function scaleY(y){
        return y/100*manager.appHeight
    }
    Style{
        id:style
    }

    function screenchange(screenname )
    {

        if(pageLoader.source === "screens/"+screenname)
            return;

        componentLoader.source = ""
        pageLoader.source = "screens/"+screenname


        //        if (pageLoader.status == Component.Ready)
        //        {
        //            manager.setDceResponse("Command to change to:" + screenname+ " was successfull")
        //        }
        //        else if (pageLoader.status == Component.Loading)
        //        {
        //            console.log("loading page from network")
        //            finishLoading(screenname)
        //        }
        //        else
        //        {
        //            console.log("Command to change to:" + screenname + " failed!")
        //            screenfile = screenname
        //            pageLoader.source = "screens/Screen_x.qml"

        //        }
    }

    function finishLoading (screenname)
    {
        if(pageLoader.status != Component.Ready)
        {
            console.log("finishing load")
            pageLoader.source = "screens/"+screenname
            console.log("screen" + screenname + " loaded.")
        }
        else
        {
            finishLoading(screenname)
        }

    }

    function checkStatus(component)
    {
        console.log(component.progress)
    }


    Loader {
        id:pageLoader
        objectName: "loadbot"
        onLoaded: {
            console.log("Screen Changed:" + pageLoader.source)
        }
        Behavior on opacity{
            PropertyAnimation{
                duration: 1000
            }
        }
        Keys.onPressed: {
            console.log(event.key)
        }
    }

    Loader{
        id:popupLoader
        anchors.centerIn: parent
        z:2
    }


    //=================Components==================================================//
    function loadComponent(componentName)
    {
        componentLoader.source = "components/"+componentName
        if (componentLoader.status == Component.Ready)
        {
            manager.setDceResponse("Command to load:" + componentName+ " was successfull")
        }
        else if (componentLoader.status == Component.Loading)
        {
            console.log("loading item from network")
            finishLoadingComponent(componentName)
        }
        else
        {
            console.log("Command to load: " + componentName + " failed!")
            componentFile = componentName
        }
    }

    function finishLoadingComponent (comp)
    {
        if(componentLoader.status != Component.Ready)
        {
            console.log("finishing network load")
            componentLoader.source = "components/"+comp
            console.log("item " + comp + " loaded.")
        }
        else
        {
            finishLoadingComponent(comp)
        }
    }


    Loader{
        id:componentLoader
        height: parent.height
        width: parent.width
        objectName: "componentbot"

    }

    PropertyAnimation{
        id:fadeout
        target:pageLoader
        properties: "opacity"; to: "0"; duration: 500
    }
    PropertyAnimation{
        id:fadein
        target:pageLoader
        properties: "opacity"; to: "1"; duration: 500
    }

}
