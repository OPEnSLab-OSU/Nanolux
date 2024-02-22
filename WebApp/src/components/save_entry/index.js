import {useEffect, useState} from "preact/hooks";
import {loadFromSlot, loadSaveSlot, saveInSlot, saveToSlot} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";

const Save_Entry = ({
    name,
    idx
}) => {
    const {isConnected} = useConnectivity();

    const save = async (event) => {
		var success = await saveToSlot(idx);
		if(!success){
			alert("Failed to save slot.");
		}
	}

	const load = async (event) => {
		await loadSaveSlot(idx);
		window.location.reload(true);
	}

    return (
        <div >
            <tr>
                <th width='150'>{name}</th>
                <th>
                    <button onClick={load}>Load Pattern</button>
                </th>
                <th>
                    <button onClick={save}>Save Pattern</button>
                </th>
                
            </tr>
        </div>
    );
}

export default Save_Entry;
