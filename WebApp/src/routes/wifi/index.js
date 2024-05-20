import {h} from 'preact';
import {joinWiFi, getHostname, saveHostname, updateLocalPassword} from "../../utils/api";
import style from "./style.css";
import WifiSelector from "../../components/wifi_selector";
import {useEffect, useState} from "preact/hooks";
import Password from "../../components/password";
import TextInput from "../../components/textinput";
import {useConnectivity} from "../../context/online_context";
import WiFiModal from "../../components/redirect_modal";
import useInterval from "../../utils/use_interval";


const Wifi = () => {
    const [currentWifi, setCurrentWifi] = useState(null);
    const [selectedWifi, setSelectedWifi] = useState(null);
    const [locked, setLocked] = useState(false);
    const [password, setPassword] = useState(null);
    const [forgetting, setForgetting] = useState(false);
    const [joining, setJoining] = useState(false);
    const [joinCompleted, setJoinCompleted] = useState(null);
    const [joinResult, setJoinResult] = useState(null);
    const [hostname, setHostname] = useState("");
    const [gotHostname, setGotHostname] = useState(false);
    const [fqdn, setFqdn] = useState(".local");
    const [isModalOpen, setIsModalOpen] = useState(false);

    const {isConnected} = useConnectivity();

    const [pass, setPass] = useState("");

    function loadHostname() {
        getHostname()
            .then(data => {
            setHostname(data.hostname);
            handleHostnameChange(data.hostname);
            setGotHostname(true);
            })
            .catch(() => setHostname(null));
    }

    useEffect(() => {
        if (isConnected) {
            loadHostname();
        }
    }, [isConnected])

    useInterval(() => {
        if (!gotHostname) {
            loadHostname();
        }
    }, 1150);



    const handleNetworkSelected = async (newWifi) => {
        // setCurrentWifi(newWifi);
        if (newWifi) {
            setSelectedWifi(newWifi)
            setLocked(newWifi.lock);
            setJoinCompleted(null);
        }
    }

    const handleWifiChanged = (newWifi) => {
            setCurrentWifi(newWifi);
    }

    const handleWiFiJoinStatus = (status) => {
        if (status === "success") {
            setJoining(false);
            setJoinResult(`Successfully joined ${selectedWifi.ssid}.`)
            setJoinCompleted(true);
            setIsModalOpen(true);
        } else if (status === "fail") {
            setJoining(false);
            setJoinResult(`Unable to join ${selectedWifi.ssid}.`)
            setJoinCompleted(true);
            setIsModalOpen(false);
        }
    }

    const handlePasswordChange = async (newValue) => {
        setPassword(newValue);
    }

    const handleJoinClick = () => {
        if (!isConnected) return;

        setJoining(true);
        setJoinCompleted(null);
        setJoinResult(null);
        const wifiKey = password || null;
        joinWiFi({ssid: selectedWifi.ssid, key: wifiKey})
            .then(response =>
            {
                setJoinCompleted(response.data.message);
            });
    };

    const handleForgetClick = () => {
        if (!isConnected) return;

        setForgetting(true);
        setJoining(false);
        setJoinCompleted(null);
        setJoinResult(null);
        joinWiFi({ssid: null, key: null})
            .then(response =>
            {
                setJoinCompleted(response.data.message);
                if (response.data.success) {
                    console.log(`Forget completed.`)
                    setCurrentWifi(null);
                    setPassword(null)
                    setForgetting(false);
                }
            });
    };

    const handleHostnameChange = (newHostname) => {
        const newName = `${newHostname}.local`
        setFqdn(newName);
    }

    const handleHostnameCommit = async (committedHostname) => {
        handleHostnameChange(committedHostname);
        if (isConnected) {
            await saveHostname(committedHostname);
        }
    }

    const handleLocalPasswordChange = async (newValue) => {

        if(newValue.length < 8){
            alert("Password is too short! Must be between 8 and 15 characters.")
            return
        }

        if(newValue.length > 15){
            alert("Password is too long! Must be between 8 and 15 characters.")
            return
        }

        setPass(newValue);
        if (isConnected) {
            await updateLocalPassword(newValue);
            alert("Password succesfully updated: please power cycle the device.")
        }
    }

    const closeModal = () => {
        setIsModalOpen(false);
    }

    return (
        <div className={style.home}>
            <div className={style.settingsControl}>
                <div className={style.centeredContainer}>
                    <TextInput
                        inputPrompt="New Local WiFi Password: "
                        commmitPrompt="Set Password"
                        textValue={pass}
                        onTextCommit={handleLocalPasswordChange}
                    />
                </div>
                <div className={style.centeredContainer}>
                    <div className={style.fqdn}>
                        Once it joins a WiFi, AudioLux can be found in the network with this name: {fqdn}
                    </div>
                </div>
            </div>
            <div className={style.settingsControl}>
                <div className={style.centeredContainer}>
                    <TextInput
                        inputPrompt="Unit Name: "
                        commmitPrompt="Save"
                        textValue={hostname}
                        onTextCommit={handleHostnameCommit}
                    />
                </div>
                <div className={style.centeredContainer}>
                    <div className={style.fqdn}>
                        Once it joins a WiFi, AudioLux can be found in the network with this name: {fqdn}
                    </div>
                </div>
            </div>
            <div className={style.settingsControl}>
                <div className={style.centeredContainer}>
                    <div>Available Networks</div>
                </div>
                <div className={style.centeredContainer}>
                    <WifiSelector placeholder="Select a network..."
                                  isJoining={joining}
                                  selectedWifi={selectedWifi}
                                  onNetworkSelected={handleNetworkSelected}
                                  onWifiChanged={handleWifiChanged}
                                  onWifiJoinStatus={handleWiFiJoinStatus}
                    />
                </div>
            </div>
            {locked && !joining && !joinCompleted &&
                <div>
                    <Password prompt="Enter WiFi key/password: "
                              password={password}
                              onPasswordChange={handlePasswordChange}
                    />
                </div>
            }
            {joining ? (
                <div>Attempting to join {selectedWifi.ssid}</div>
            ):(
                <button className={style.formButton}
                        onClick={handleJoinClick}
                        disabled={!joining && selectedWifi?.ssid == null}
                >Join</button>
                )
            }
            {joinCompleted && !joining &&
                <div>
                    {joinResult}
                </div>
            }
            <div className={style.settingsControl}>
                <div className={style.wifiBanner}>
                    {forgetting ? "Fogetting WiFi" : "Current Wifi" }: {currentWifi?.ssid ?? "None"}
                </div>
                <button className={style.formButton}
                        disabled={forgetting || !(currentWifi?.ssid)}
                        onClick={handleForgetClick}
                >Forget
                </button>
            </div>
            <WiFiModal
                isOpen={isModalOpen}
                onClose={closeModal}
                ssid={currentWifi?.ssid}
                audioLuxUrl={fqdn}
            />
        </div>
    );
};

export default Wifi;
