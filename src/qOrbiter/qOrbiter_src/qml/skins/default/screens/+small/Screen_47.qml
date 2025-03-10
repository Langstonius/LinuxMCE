import QtQuick 2.4
import org.linuxmce.enums 1.0
import "../../components"
import "../."

StyledScreen {
    id:mediaFilesView
    screen:qsTr("Media Files View")
    onActiveFocusChanged:{
        if(activeFocus) innerContent.forceActiveFocus();
    }
    onScreenOpening: {
        manager.setStringParam(0, manager.q_mediaType)
    }


    Component{
        id:filedetailscomp
        FileDetails {
            id: fileDetails
        }
    }

    Connections{
        target: filedetailsclass
        onShowDetailsChanged:{
            qmlRoot.createPopup(filedetailscomp)
        }
    }

    Item{
        id:innerContent
        anchors.fill: parent

        state:manager.q_mediaType =="5" ? "entry" : "selection"
        onActiveFocusChanged: {
            if(!activeFocus)
                return

            if(state=="entry")
                typeSelection.forceActiveFocus()
            else if(state=="selection")
                multi_view_list.forceActiveFocus()
            else if(state=="filter")
                console.log("filter focus")
        }


        MultiMediaView {
            id: multi_view_list
            onOptions: option_row.forceActiveFocus()
            anchors{
                top:option_row.bottom
                bottom: parent.bottom
                left:parent.left
                right:parent.right
            }
        }
        MediaOptionRow {
            id: option_row
        }
        Loader{
            id:a_z
            anchors{
                top:option_row.bottom
                right: parent.right
                bottom:parent.bottom
            }
        }
        Component{
            id:alpha
            Alphabetlist{
                height: a_z.height
               onAlphaSelected: multi_view_list.seek(selectedAlpha)
            }
        }
        LargeStyledButton{
            id:file_select_btn
            anchors{
                top:parent.top
                topMargin:appStyle.scaleY(15)
                left:typeSelection.left
                right: typeSelection.right
            }
            height:appStyle.scaleY(8)
            buttonText: qsTr("Browse By File")
            visible: typeSelection.visible
            onActivated: {
                attribfilter.setSelectionStatus("File")
                innerContent.state="selection"
            }

        }

        GridView{
            id:typeSelection
            Component.onCompleted: innerContent.forceActiveFocus()
            onActiveFocusChanged: console.log("media focus")
            anchors{
                top:file_select_btn.bottom
                topMargin:5
                horizontalCenter: parent.horizontalCenter
            }

            height:parent.height / 2
            width: parent.width *.95
            model:mediatypefilter
            cellWidth:typeSelection.width/3
            cellHeight:manager.isProfile ? appStyle.scaleY(15) : appStyle.scaleY(22)
            cacheBuffer:50
            delegate:
                LargeStyledButton{
                height:typeSelection.cellHeight -5
                width: typeSelection.cellWidth -5
                arrow: false
                buttonText: name
                onActivated: {
                    mediatypefilter.setSelectionStatus(name); innerContent.state="selection"
                }
            }
        }


        states: [
            State {
                name: "entry"
                PropertyChanges {
                    target: typeSelection
                    visible:true
                }
                PropertyChanges {
                    target: multi_view_list
                    visible:false
                }
                PropertyChanges {
                    target: option_row
                    visible:false
                }
                StateChangeScript{
                    script:typeSelection.forceActiveFocus()
                }
            },
            State {
                name: "selection"
                PropertyChanges {
                    target: typeSelection
                    visible:false
                }
                PropertyChanges {
                    target: option_row
                    visible:true
                }
                PropertyChanges {
                    target: multi_view_list
                    visible:true
                }
                StateChangeScript{
                    script:{multi_view_list.forceActiveFocus(); multi_view_list.load()}
                }
            },
            State {
                name: "filter"
                extend: "entry"
            },
            State {
                name: "aplha"
                extend: "selection"
            }
        ]
    }


}
