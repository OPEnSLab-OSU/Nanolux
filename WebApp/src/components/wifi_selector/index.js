import {h} from 'preact';
import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import {getWiFi, getWiFiList} from "../../utils/api";

const WifiSelector = ({onNetworkSelected}) => {
    const [wifiList, setWifiList] = useState([]);
    const [currentWifi, setCurrentWifi] = useState("")

    useEffect(() => {
        getWiFiList().then(data => setWifiList(data));
        getWiFi().then(data => setCurrentWifi(data));
    }, [])

    function rssi_icon(rssi) {
        return " ";
    }

    function lock_icon(encryption) {
        return " ";
    }

    const availableWifis = wifiList.map(wifi => {
        return <option key={wifi.ssid} value={wifi.ssid}>
            {wifi.ssid} {rssi_icon(wifi.rssi)} {lock_icon(wifi.encryption)}
        </option>
    });

    const handleSelection = async (event) => {
        const newWiFi = event.target.value;
        setCurrentWifi(newWiFi);
        onNetworkSelected(newWiFi);
    }

    return (
        <div>
            <label className={style.label} htmlFor="wifi-options">Current Pattern</label>
            <select className={style.label}
                    id="wifi-options"
                    value={currentWifi ?? "Select a network"}
                    onChange={handleSelection}
            >
                {availableWifis}
            </select>
        </div>
    );
}

export default WifiSelector;
