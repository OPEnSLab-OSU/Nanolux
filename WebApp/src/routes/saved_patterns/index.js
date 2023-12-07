import style from './style.css';
import Patterns from "../../components/patterns";
import ModeSelector from "../../components/mode_selector";
import SecondaryPatterns from "../../components/secondary_patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {getNoise, saveNoise, getAlpha, saveAlpha} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import Save_Entry from '../../components/save_entry';

const Saves = () => {
	const { isConnected } = useConnectivity();

	const [settings, setSettings] = useState({});

	useEffect(() => {
		if (isConnected) {
		}
	}, [isConnected])


	return (
		<div className={style.home}>
            <Save_Entry 
                name="Default Pattern"
                idx='0'
            />
            <Save_Entry 
                name="Saved Pattern 1"
                idx='1'
            />
            <Save_Entry 
                name="Saved Pattern 2"
                idx='2'
            />
            <Save_Entry 
                name="Saved Pattern 3"
                idx='3'
            />
            <Save_Entry 
                name="Saved Pattern 4"
                idx='4'
            />
		</div>
	);
};

export default Saves;