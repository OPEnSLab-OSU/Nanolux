import {h} from 'preact';
import {joinWiFi, getHostname, saveHostname} from "../../utils/api";
import style from "./style.css";
import WifiSelector from "../../components/wifi_selector";
import {useEffect, useState} from "preact/hooks";
import Password from "../../components/password";
import TextInput from "../../components/textinput";
import {useConnectivity} from "../../context/online_context";
import WiFiModal from "../../components/redirect_modal";


const Wifi = () => {
    const [currentWifi, setCurrentWifi] = useState(null);
    const [selectedWifi, setSelectedWifi] = useState(null);
    const [locked, setLocked] = useState(false);
    const [password, setPassword] = useState(null);
    const [forgetting, setForgetting] = useState(false);
    const [joining, setJoining] = useState(false);
    const [joinCompleted, setJoinCompleted] = useState(null);
    const [hostname, setHostname] = useState("");
    const [fqdn, setFqdn] = useState(".local");
    const [isModalOpen, setIsModalOpen] = useState(false);

    const {isConnected} = useConnectivity();


    useEffect(() => {
        if (isConnected) {
            getHostname().then(data => {
                setHostname(data.hostname);
                handleHostnameChange(data.hostname);
            });
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
            if (joining && selectedWifi?.ssid != null && newWifi.ssid === selectedWifi.ssid) {
                setJoining(false);
                setJoinCompleted(true);
                setIsModalOpen(true);
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
            });
    };

    const handleForgetClick = () => {
        if (!isConnected) return;

        setForgetting(true);
        setJoining(false)
        setJoinCompleted(null)
        joinWiFi({ssid: null, key: null})
            .then(response =>
            {
                setJoinCompleted(response.data.message);
                if (response.data.success) {
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

    const closeModal = () => {
        setIsModalOpen(false);
    }

    return (
        <div className={style.home}>
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
                                  onNetworkSelected={handleNetworkSelected}
                                  onWifiChanged={handleWifiChanged}
                    />
                </div>
            </div>            {locked && !joining &&
                <div>
                    <Password prompt="Enter WiFi key/password: "
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
            {joinCompleted &&
                <div>
                    {joinCompleted}
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
