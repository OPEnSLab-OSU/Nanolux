import {h} from 'preact';
import {joinWiFi, getHostname, saveHostname} from "../../utils/api";
import style from "./style.css";
import WifiSelector from "../../components/wifi_selector";
import {useEffect, useState} from "preact/hooks";
import Password from "../../components/password";
import TextInput from "../../components/textinput";
import {useConnectivity} from "../../context/online_context";


const Wifi = () => {
    const [currentWifi, setCurrentWifi] = useState(null);
    const [selectedWifi, setSelectedWifi] = useState(null);
    const [locked, setLocked] = useState(false);
    const [password, setPassword] = useState(null);
    const [joinCompleted, setJoinCompleted] = useState(null);
    const [joining, setJoining] = useState(false);
    const [hostname, setHostname] = useState("");
    const [fqdn, setFqdn] = useState(".local");

    const {isConnected} = useConnectivity();


    useEffect(() => {
        if (isConnected) {
            getHostname().then(data => {
                setHostname(data.hostname);
                handleHostnameChange(data.hostname);
            });
            // getWiFi().then(data => setCurrentWifi(data));
        }
    }, [isConnected])


    const handleNetworkSelected = async (newWifi) => {
        // setCurrentWifi(newWifi);
        if (newWifi) {
            setSelectedWifi(newWifi)
            setLocked(newWifi.lock);
            setJoinCompleted(null);
        }
    }

    const handleWifiChanged = (newWifi) => {
        if (newWifi) {
            setCurrentWifi(newWifi);
            if (joining) {
                setJoinCompleted(true);
                setJoining(false);
                console.log('Inside sets/wifi/handleWiFiChanged. Join Completed)');
            }
        }
    }

    const handlePasswordChange = async (newValue) => {
        setPassword(newValue);
    }

    const handleJoinClick = () => {
        if (!isConnected) return;

        setJoining(true)
        setJoinCompleted(null)
        const wifiKey = password || null;
        joinWiFi({ssid: selectedWifi.ssid, key: wifiKey})
            .then(response =>
            {
                setJoinCompleted(response.data.message);
                if (!response.data.success) {
                    // setCurrentWifi(selectedWifi);
                }
            });
    };

    const handleForgetClick = () => {
        if (!isConnected) return;

        setJoining(false)
        setJoinCompleted(null)
        joinWiFi({ssid: null, key: null})
            .then(response =>
            {
                setJoinCompleted(response.data.message);
                if (response.data.success) {
                    setCurrentWifi(null);
                    setPassword(null)
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

    return (
        <div className={style.home}>
            <div className={style.settingsControl}>
                <TextInput
                    inputPrompt="Hostname: "
                    commmitPrompt="Save"
                    textValue={hostname}
                    onTextCommit={handleHostnameCommit}
                />
                <div className={style.fqdn}>
                    Current full network name: {fqdn}
                </div>
            </div>
            <div className={style.settingsControl}>
                <div>Available Networks</div>
                <div>
                    <WifiSelector placeholder="Select a network..."
                                  onNetworkSelected={handleNetworkSelected}
                                  onWifiChanged={handleWifiChanged}
                    />
                </div>
            </div>
            {locked &&
                <div>
                    <Password prompt="Enter WiFi key/password: "
                              onPasswordChange={handlePasswordChange}
                    />
                </div>
            }
            {
                <button className={style.formButton}
                        onClick={handleJoinClick}
                        disabled={joining && selectedWifi != null}
                >Join</button>
            }
            {joinCompleted &&
                <div>
                    joinCompleted
                </div>
            }
            <div className={style.settingsControl}>
                <div className={style.wifiBanner}>
                    Current Wifi: {currentWifi?.ssid ?? "None"}
                </div>
                <button className={style.formButton}
                        disabled={!(currentWifi?.ssid)}
                        onClick={handleForgetClick}
                >Forget
                </button>
            </div>
        </div>
    );
};

export default Wifi;
