import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	getLoadedSubpattern,
	updateLoadedSubpattern,
	getLoadedPatternSettings,
	updateLoadedPattern,
	modifyLoadedSubpatternCount,
	getPatternList,
	getLoadedSubpatternCount} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";

const Subpattern = ({subpattern, patterns}) => {

	const { isConnected } = useConnectivity();
	const [loading, setLoading] = useState(true);
	const [updated, setUpdated] = useState(false);

	// Subpattern data structure
	const [data, setData] = useState({
		idx: 0,
		hue_max: 0,
		hue_min: 255,
		brightness: 255,
		smoothing: 0,
		direction: 0,
		noise: 20
	});

	// Manage initalization
	useEffect(() => {
        if (isConnected) {
            getLoadedSubpattern(subpattern).then(data => {setData(data)}).then();
			setLoading(false)
        }  
    }, [isConnected])

	// Send out an update whenever something is changed
	useInterval(() => {
        if (isConnected && updated) {
            updateLoadedSubpattern(subpattern, data);
			setUpdated(false);
        }
    }, 100);

	// Update any parameters of the subpattern.
	const update = (ref, value) => {
		if(!loading){		
			setData((oldData) => {
				const newData = Array.from(oldData);
				newData[ref] = value;
				return newData;
			})
		}
		setUpdated(true);
	}

	// Generate the subpattern structure.
	return (
		(!loading ? 
		<div>
			<Patterns
				initialID={data.idx}
				structure_ref={"idx"}
				update={update}
				patterns={patterns}
			/>
			<NumericSlider
				className={style.settings_control}
				label="Noise Threshold"
				min={0}
				max={100}
				initial={data.noise}
				structure_ref="noise"
				update={update}
			/>
			<NumericSlider
				className={style.settings_control}
				label="Brightness"
				min={0}
				max={255}
				initial={data.brightness}
				structure_ref="brightness"
				update={update}
			/>
			<NumericSlider
				className={style.settings_control}
				label="Smoothing"
				min={0}
				max={175}
				initial={data.smoothing}
				structure_ref="smoothing"
				update={update}
			/>
		</div> : <div></div>
		)
	);
}

const CurrentPattern = ({initialSubpatternCount, patterns}) => {

	const { isConnected } = useConnectivity();
	const [loading, setLoading] = useState(true);
	const [updated, setUpdated] = useState(false);

	// Subpattern data structure
	const [data, setData] = useState({
		subpattern_count: initialSubpatternCount,
		alpha: 0,
		mode: 0
	});

	const [selectedSubpattern, setSubpattern] = useState(0);

	// Manage initalization
	useEffect(() => {
        if (isConnected) {
            getLoadedPatternSettings().then(data => setData(data));
            setLoading(false);
        }  
    }, [isConnected])

	// Send out an update whenever something is changed
	useInterval(() => {
        if (isConnected && updated) {
            updateLoadedPattern(data);
			setUpdated(false);
        }
    }, 100);

	// Update any parameters of the subpattern.
	const update = (ref, value) => {
		if(!loading){		
			setData((oldData) => {
				const newData = Array.from(oldData);
				newData[ref] = value;
				return newData;
			})
		}
		
		setUpdated(true);
	}

	const incrementSubpatterns = async () => {
		if(data.subpattern_count < 4){
			await modifyLoadedSubpatternCount(1);
			update("subpattern_count", data.subpattern_count + 1);
		}
	}

	const decrementSubpatterns = async () => {
		if(data.subpattern_count > 1){
			await modifyLoadedSubpatternCount(-1);
			update("subpattern_count", data.subpattern_count - 1)
		}
	}

	function inRange(end){
		var l = [];
		for(var i = 0; i < end; i++){
			l.push({idx: i});
		}
		return l;
	}

	return (
		<div>
			<NumericSlider
				className={style.settings_control}
				label="Transparency"
				min={0}
				max={255}
				initial={data.alpha}
				structure_ref="alpha"
				update={update}
			/>
			<NumericSlider
				className={style.settings_control}
				label="Mode"
				min={0}
				max={1}
				initial={data.mode}
				structure_ref="mode"
				update={update}
			/>

			<button type="button" onClick={incrementSubpatterns}>+</button>
			<button type="button" onClick={decrementSubpatterns}>-</button>
			<br></br>

			{inRange(data.subpattern_count).map((data) => {
				return <button type="button" onClick={function() {setSubpattern(data.idx);}} key={data.idx}>
					SP {data.idx}
				</button>
			})}

			<Subpattern
				subpattern={selectedSubpattern}
				patterns={patterns}
				key={selectedSubpattern}
			/>
				
		</div>
	);
}

const Settings = () => {

	const {isConnected} = useConnectivity();
	const [patterns, setPatterns] = useState([]);
	const [loadingPatterns, setLoading] = useState(true);
	const [loadingSpc, setSpcLoading] = useState(true);

	const [spc, setSpc] = useState(1);

	useEffect(() => {
        if (isConnected) {
            getPatternList().then(data => {setPatterns(data); setLoading(false);});
			getLoadedSubpatternCount().then(data => {setSpc(data); setSpcLoading(false);})
        }
    }, [isConnected])

	return (

		(loadingPatterns && loadingSpc ? <div></div> :
			<div>
				<CurrentPattern
					initialSubpatternCount={1}
					patterns={patterns}
				/>
			</div>
		)
	);
};

export default Settings;
