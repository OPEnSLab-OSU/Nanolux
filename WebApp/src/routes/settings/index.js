import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import SimpleSlider from "../../components/simple_slider";

const Settings = () => {
	return (
		<div className={style.home}>
			<Patterns patterns={["Pattern1", "Pattern2", "Pattern3"]} />
			<NumericSlider label="Noise Threshold" />
			<NumericSlider label="Compression threshold" />
			<SimpleSlider label="Low Frequency Color" />
			<SimpleSlider label="High Frequency Color" />
		</div>
	);
};

export default Settings;
