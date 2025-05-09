import style from './style.css';
import {useState, useEffect} from "preact/hooks";
import {useConnectivity} from "../../context/online_context";
import BasicSettings from '../../components/basic_settings';
import { getPatternList } from '../../utils/api';

/**
 * @brief Generates the main settings page for the settings route.
 */
const Test = () => {

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
					<th>Settings</th>	
				</tr>
				<tr>
					<td>
						<BasicSettings
							patterns={patterns}
						/>
					</td>					
				</tr>
			</table>
		</div>
	);
};

export default Test;
