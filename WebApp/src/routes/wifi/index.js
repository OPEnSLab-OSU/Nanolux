import {h} from 'preact';
import {getWiFi, joinWiFi, getHostname, saveHostname} from "../../utils/api";
import style from "./style.css";
import WifiSelector from "../../components/wifi_selector";
import {useEffect, useState} from "preact/hooks";
import Password from "../../components/password";
import TextInput from "../../components/textinput";
import {useModal} from "../../context/global_modal_context";
import {useConnectivity} from "../../context/online_context";


const Wifi = () => {
    const [currentWifi, setCurrentWifi] = useState(null);
    const [selectedWifi, setSelectedWifi] = useState(null);
    const [locked, setLocked] = useState(false);
    const [password, setPassword] = useState("");
    const [joinCompleted, setJoinCompleted] = useState(null);
    const [hostname, setHostname] = useState("");
    const [fqdn, setFqdn] = useState(".local");

    const {openModal} = useModal()
    const {isConnected} = useConnectivity();


    useEffect(() => {
        if (isConnected) {
            getHostname().then(data => {
                setHostname(data.hostname);
                handleHostnameChange(data.hostname);
            });
            getWiFi().then(data => setCurrentWifi(data));
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

    const handlePasswordChange = async (newValue) => {
        setPassword(newValue);
    }

    const handleJoinClick = () => {
        if (!isConnected) return;

        setJoinCompleted(null)
        joinWiFi({ssid: selectedWifi.ssid, key: password})
            .then(response =>
            {
                setJoinCompleted(response.data.message);
                if (response.data.success) {
                    setCurrentWifi(selectedWifi);
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
                >Join</button>
            }
            {joinCompleted &&
                <div>
                    {openModal}
                    {joinCompleted}
                </div>
            }
            {currentWifi &&
                <div className={style.settingsControl}>
                    <div className={style.wifiBanner}>
                        Current Wifi: {currentWifi.ssid}
                    </div>
                    <button className={style.formButton}
                            onClick={handleJoinClick}
                    >Forget
                    </button>
                </div>
            }
        </div>
    );
};

export default Wifi;
