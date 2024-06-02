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

/**
 * @brief The object that generates the WiFi options page.
 */
const Wifi = () => {

    /// Controls the modal instance for the Wifi page.
    const [isModalOpen, setIsModalOpen] = useState(false);
    
    /// Stores the text for the current AudioLux MDNS address.
    const [fqdn, setFqdn] = useState(".local");

    /// Stores the currently-connected Wifi connection.
    const [currentWifi, setCurrentWifi] = useState(null);

    /// Stores the Wifi connection that the user has selected,
    /// but (possibly) not set.
    const [selectedWifi, setSelectedWifi] = useState(null);

    /// State that prevents the user from accessing the element
    /// that allows the user to enter a password for the Wifi.
    const [locked, setLocked] = useState(false);

    /// Currently-stored Wifi password the user has entered.
    const [password, setPassword] = useState(null);

    /// Is set to "true" when the device is in the process of
    /// forgetting the current Wifi settings.
    const [forgetting, setForgetting] = useState(false);

    /// Is set to "true" when the device is in the process of
    /// joining a Wifi connection.
    const [joining, setJoining] = useState(false);

    /// Is set to "true" when the join process has been completed.
    const [joinCompleted, setJoinCompleted] = useState(null);

    /// Stores if the join process succeeded or failed.
    const [joinResult, setJoinResult] = useState(null);

    /// Stores the current hostname of the device (user-configurable)
    const [hostname, setHostname] = useState("");

    /// Is "true" when the hostname of the device has been obtained.
    const [gotHostname, setGotHostname] = useState(false);
    
    /// Stores the state of device connectivity.
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

    /**
     * Loads the hostname of the device when the app is connected.
     */
    useEffect(() => {
        if (isConnected) {
            loadHostname();
        }
    }, [isConnected])

    /**
     * Loads the hostname of the device when the app is connected.
     */
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
