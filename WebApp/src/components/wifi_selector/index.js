import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getWiFi, getWiFiList} from "../../utils/api";
import FaWifi from "../icons/wifi";
import FaWifiWeak from "../icons/wifi-weak";
import FaWifiFair from "../icons/wifi-fair";
import FaLock from "../icons/lock";
import FaUnlock from "../icons/unlock";
import FaArrowDown from "../icons/arrow-down";
import useInterval from "../../utils/use_interval";


// https://medium.com/tinyso/how-to-create-a-dropdown-select-component-in-react-bf85df53e206
const WifiSelector = ({
                          placeholder,
                          onNetworkSelected,
                          onWifiChanged
                      }) => {
    const [wifiList, setWifiList] = useState([]);
    const [showMenu, setShowMenu] = useState(false);
    const [selectedWifi, setSelectedWifi] = useState(null)
    const [currentWifi,  setCurrentWifi] = useState(null);

    let key = 1;

    const getWifiInfo = () => {
        getWiFiList().then(data => setWifiList(data));
        getWiFi().then(data => {
            if (!currentWifi || currentWifi.ssid !== data.ssid) {
                setCurrentWifi(data);
            }
        });
    }

    useInterval(() => {
        getWifiInfo();
        }, 15000);

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
        // onNetworkSelected()
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
        return selectedWifi ? selectedWifi.ssid : placeholder;
    }

    const handleItemClick = (selectedWifi) => {
        setSelectedWifi(selectedWifi);
        onNetworkSelected(selectedWifi);
    }

    return (
        <div>
            <div className={style.dropdownContainer}>
                <div className={style.dropdownInput}
                     onClick={handleInputClick}>
                    <div className={style.dropDownSelectedValue}>{getDisplay()}</div>
                    <div className={style.dropDownTools}>
                        <div className={style.dropDownTool}>
                            <FaArrowDown />
                        </div>
                    </div>
                </div>
                {showMenu &&
                    <div className={style.dropdownMenu}>
                        {wifiList.map(wifi => (
                            <div key={key++}
                                 className={style.dropdownItem}
                                 onClick={() => handleItemClick(wifi)}
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
        </div>
    );
}

export default WifiSelector;
