import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getWiFi, getWiFiJoinStatus, getWiFiList} from "../../utils/api";
import FaWifi from "../icons/wifi";
import FaWifiWeak from "../icons/wifi-weak";
import FaWifiFair from "../icons/wifi-fair";
import FaLock from "../icons/lock";
import FaUnlock from "../icons/unlock";
import FaArrowDown from "../icons/arrow-down";
import useInterval from "../../utils/use_interval";
import {LabelSpinner, Spinner} from "../spinner";
import {useConnectivity} from "../../context/online_context";


// https://medium.com/tinyso/how-to-create-a-dropdown-select-component-in-react-bf85df53e206
const WifiSelector = ({
                          placeholder,
                          isJoining,
                          selectedWifi,
                          onNetworkSelected,
                          onWifiChanged,
                          onWifiJoinStatus
                      }) => {
    const [wifiList, setWifiList] = useState([]);
    const [showMenu, setShowMenu] = useState(false);
    const [currentWifi,  setCurrentWifi] = useState(null);
    const [loading, setLoading] = useState(true);
    const [isScanning, setIsScanning] = useState(false);
    const [timerDelay, setTimerDelay] = useState(11000);
    const {isConnected} = useConnectivity();

    let key = 1;
    let wifiListErrors = 0;

    const time = () => {
        const now = new Date();
        return now.toLocaleTimeString();
    }

    useEffect(() => {
        getWifiInfo();
    }, []);

    const getWifiInfo = () => {
        console.log(time(), `Inside getWifiInfo. isJoining: ${isJoining}, isScanning: ${isScanning}`);
        if (isConnected) {
            if (isJoining) {
                getWiFiJoinStatus().then(data => onWifiJoinStatus && onWifiJoinStatus(data.status));
            } else if (isScanning) {
                console.log(time(), 'second getWiFiList call');
                getWiFiList().then(data => {
                    if (data && data.length > 0) {
                        setWifiList(data);
                        setIsScanning(false);
                    } else {
                        wifiListErrors++;
                    }
                }).catch((error) => {
                        console.log(time(), `WifiList call failed: ${error}. Will try again.`);
                        wifiListErrors++;
                });
                if (wifiListErrors > 3) {
                    setIsScanning(false);
                    console.log(time(), `Unable to get a scan from device.`);
                }
            } else {
                getWiFi().then(data => {
                    if (!currentWifi || currentWifi.ssid !== data.ssid) {
                        setCurrentWifi(data);
                    }
                });
                setLoading(false);
            }
        }
    }

    const getWifiList = () => {
        if (isConnected) {
            console.log(time(), 'first getWiFiList call.');
            wifiListErrors = 0;
            setIsScanning(true)
            getWifiInfo();
        }
    }

    useEffect(() => {
        console.log(time(), `Inside mode useEffect. isJoining: ${isJoining}, isScanning: ${isScanning}`);
        if (isJoining) {
            setTimerDelay(500);
        } else if (isScanning) {
            setTimerDelay(8700);
        } else {
            setTimerDelay(11300);
        }
    }, [isJoining, isScanning]);

    useInterval(() => {
        console.log(time(), 'getWifiInfo invoked from useInterval');
        getWifiInfo();
    }, timerDelay);

    useEffect(() => {
        if (onWifiChanged) {
            onWifiChanged(currentWifi)
        }
    }, [onWifiChanged, currentWifi]);

    useEffect(() => {
        const handler = () => setShowMenu(false);
        window.addEventListener("click", handler);

        return () => {
            window.removeEventListener("click", handler);
        }
    });

    const handleInputClick = (event) => {
        event.stopPropagation();
        setShowMenu(!showMenu);
    };

    function rssi_icon(rssi) {
        if (rssi >= -20) {
            return <FaWifi />
        } else if (rssi >= -50) {
            return <FaWifiFair />
        }
        return <FaWifiWeak />
    }

    function lock_icon(lock) {
        if (lock) {
            return <FaLock />;
        }
        return <FaUnlock />
    }

    const getDisplay = () => {
        if (isScanning) {
            return "Scanning...";
        }
        return selectedWifi ? selectedWifi.ssid : placeholder;
    }

    const handleItemClick = (selectedWifi) => {
        onNetworkSelected(selectedWifi);
    }

    return (
        <div>
            {loading ? (
                <LabelSpinner />
            ) : (
                <div className={style.selector}>
                <div className={`${style.dropdownContainer} ${style.leftComponent}`}>
                    <div
                      className={style.dropdownInput}
                      id='wifi-combobox'
                      role='combobox'
                      aria-expanded={showMenu}
                      aria-haspopup='listbox'
                      aria-controls='wifi-list'
                      tabIndex={0}
                      aria-label='Select WiFi network'
                      onKeyDown={e => {
                        if (e.key === 'Enter' || e.key === ' ') handleInputClick();
                      }}
                      onClick={handleInputClick}
                    >
                        <div className={style.dropDownSelectedValue}>{getDisplay()}</div>
                        <div className={style.dropDownTools}>
                            <div className={style.dropDownTool}>
                                {isScanning ? (
                                    <Spinner />
                                ) : (
                                    <FaArrowDown aria-hidden='true' />
                                ) }
                            </div>
                        </div>
                    </div>
                    {showMenu &&
                        <div className={style.dropdownMenu} id='wifi-list' role='listbox' aria-labelledby='wifi-combobox'>
                            {wifiList.map(wifi => (
                                <div
                                  key={key++}
                                  className={style.dropdownItem}
                                  role='option'
                                  aria-selected={selectedWifi ? wifi.ssid === selectedWifi.ssid : false}
                                  tabIndex={0}
                                  onClick={() => handleItemClick(wifi)}
                                  onKeyDown={e => {
                                    if (e.key === 'Enter' || e.key === ' ') {
                                        e.preventDefault();
                                        handleItemClick(wifi);
                                    }
                                  }}
                                >
                                    <div className={style.dropdownItemSSID}>{wifi.ssid}</div>
                                    <div className={style.dropdownItemRSSI}>{rssi_icon(wifi.rssi)}</div>
                                    <div className={style.dropdownItemLock}>{lock_icon(wifi.lock)}</div>
                                </div>
                            ))
                            }
                        </div>
                    }
                </div>
                    <button className={`${style.scanButton} ${style.rightComponent}`} onClick={getWifiList} aria-label='Scan for WiFi networks'>Scan</button>
                </div>
            )}
        </div>
    );
}

export default WifiSelector;
