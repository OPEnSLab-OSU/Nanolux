import style from './style.css';
import {useState, useEffect} from "preact/hooks";
import {useConnectivity} from "../../context/online_context";
import Save_Entry from '../../components/save_entry';
import StripSettings from '../../components/strip_settings';
import SystemControls from '../../components/system_settings';
import { getPatternList } from '../../utils/api';

/**
 * @brief Generates the main settings page for the settings route.
 */
const Settings = () => {

	// Object that returns if the app is connected to the device.
	const { isConnected } = useConnectivity();

	// Stores the list of patterns obtained from the device.
	const [patterns, setPatterns] = useState([{index: 0, name: "None"}])

	/**
	 * @brief Obtains the list of patterns from the NanoLux device.
	 */
	useEffect(() => {
        if (isConnected) {
            getPatternList()
				.then(data => setPatterns(data))
				.then();
        }  
    }, [isConnected])

	return (
		<div>
			<table>
				<tr>
					<th style="font-size: 1.6rem;">Pattern Settings</th>
					<th style="font-size: 1.6rem;">System Settings</th>	
				</tr>
				<tr>
					<td>
						<StripSettings
							patterns={patterns}
						/>
					</td>
					<td>
						<SystemControls/>
						<hr></hr>
						<br/>
						<div className={style.background0}>
							<Save_Entry 
								name="Default Pattern"
								idx='0'
							/>
						</div>
						<div className={style.background1}>
							<Save_Entry
								name="Saved Pattern 1"
								idx='1'
							/>
						</div>
						<div className={style.background2}>
							<Save_Entry
								name="Saved Pattern 2"
								idx='2'
							/>
						</div>
					</td>
					
				</tr>
			</table>
		</div>
	);
};

export default Settings;
