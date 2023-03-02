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

// https://medium.com/tinyso/how-to-create-a-dropdown-select-component-in-react-bf85df53e206
const WifiSelector = ({
    placeholder,
    onNetworkSelected
}) => {
    const [wifiList, setWifiList] = useState([]);
    const [currentWifi, setCurrentWifi] = useState(null);
    const [showMenu, setShowMenu] = useState(false);
    let key = 1;

    useEffect(() => {
        getWiFiList().then(data => setWifiList(data));
        getWiFi().then(data => setCurrentWifi(data));
    }, [])

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
        onNetworkSelected()
    };

    function rssi_icon(rssi) {
        if (rssi >= 80) {
            return <FaWifi />
        } else if (rssi >= 50) {
            return <FaWifiFair />
        }
        return <FaWifiWeak />
    }

    function lock_icon(lock) {
        if (lock) {
            return <FaLock  />;
        }
        return <FaUnlock />
    }

    const getDisplay = () => {
        if (currentWifi) {
            return currentWifi.ssid;
        }

        return placeholder;
    }

    const handleItemClick = (selectedWifi) => {
        setCurrentWifi(selectedWifi);
        onNetworkSelected(selectedWifi);
    }

    const isSelected = (option) => {
        if (!currentWifi) {
            return false;
        }

        return currentWifi.value === option.value;
    }

    return (
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
                {   wifiList.map(wifi => (
                    <div key={key++}
                         className={`dropdownItem ${isSelected(wifi) && "dropdownItem.selected"}`}
                         onClick={() => handleItemClick(wifi)}
                    >
                        {wifi.ssid} {rssi_icon(wifi.rssi)} {lock_icon(wifi.lock)}
                    </div>
                    ))
                }
            </div>
            }
        </div>
    );
}

export default WifiSelector;
