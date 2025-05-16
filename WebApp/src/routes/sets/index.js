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

	// Toggle state for showing the advanced settings
	const [showAdvanced, setShowAdvanced] = useState(false);

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
		<div className={style.settingsPage}>
			{/* BASIC SETTINGS */}
			{!showAdvanced && (
				<div className={style.basicSection}>
					<label style={{ fontSize: '1.6rem', fontWeight: 'bold' }}>Pattern Settings</label>
					<br/><br/>
					<StripSettings patterns={patterns} advanced={false} />
				</div>
		  	)}

			{/* FULL SETTINGS */}
		  	{showAdvanced && (
				<div className={style.advancedSection}>
			  		<div className={style.advancedLeft}>
						<label style={{ fontSize: '1.6rem', fontWeight: 'bold' }}>Pattern Settings</label>
						<br/><br/>
						<StripSettings patterns={patterns} advanced={true} />
			  		</div>
			  		<div className={style.advancedRight}>
			  			<label style={{ fontSize: '1.6rem', fontWeight: 'bold' }}>System Settings</label>
			  			<br/><br/><br/>
						<SystemControls />
						<hr/>
						<br/>
						<div className={style.background0}>
				  			<Save_Entry name="Default Pattern" idx="0" />
						</div>
						<div className={style.background1}>
				  			<Save_Entry name="Saved Pattern 1" idx="1" />
						</div>
						<div className={style.background2}>
				  			<Save_Entry name="Saved Pattern 2" idx="2" />
						</div>
			  		</div>
				</div>
		  	)}
	
		  	{/* ADVANCED TOGGLE BUTTON */}
		  	<button
				className={style.advancedToggle}
				onClick={() => setShowAdvanced(!showAdvanced)}
		  	>
				{showAdvanced ? 'Hide Advanced' : 'Show Advanced'}
		  	</button>
		</div>
	);
};

export default Settings;
