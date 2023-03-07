import { h } from 'preact';
import {joinWiFi} from "../../utils/api";
import style from "./style.css";
import WifiSelector from "../../components/wifi_selector";
import {useState} from "preact/hooks";
import Password from "../../components/password";

const Wifi = () => {
	const [currentWifi, setCurrentWifi] = useState(null);
	const [locked, setLocked] = useState(false)
	const [password, setPassword] = useState("")
	const [joinCompleted, setJoinCompleted] = useState(null)

	const handleNetworkSelected = async (newWifi) => {
		setCurrentWifi(newWifi);
		if (newWifi) {
			setLocked(newWifi.lock);
		}
	}

	const handlePasswordChange = async (newValue) => {
		setPassword(newValue);
	}

	const handleJoinClick = () => {
		setJoinCompleted(null);
		joinWiFi({ssid: currentWifi.ssid, key: password})
			.then(response => setJoinCompleted(response.data.message))

	};

	return (
		<div className={style.home}>
			<div>Available Networks</div>
			<div className={style.settingsControl}>
				<WifiSelector placeholder="Select a network..." onNetworkSelected={handleNetworkSelected} />
			</div>
			{ locked &&
				<div>
				<Password prompt="Enter WiFi key/password"
						  onPasswordChange={handlePasswordChange}
				/>
				</div>
			}
			{ currentWifi &&
				<div>
					<button className={style.sendButton}
							onClick={handleJoinClick}
					>{joinCompleted ? "Forget" : "Join"}</button>
				</div>
			}
			{ joinCompleted &&
				<div>
					{joinCompleted}
				</div>
			}
		</div>
	);
};

export default Wifi;
